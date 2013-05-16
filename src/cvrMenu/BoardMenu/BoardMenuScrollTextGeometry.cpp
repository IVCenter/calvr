#include <cvrMenu/BoardMenu/BoardMenuScrollTextGeometry.h>
#include <cvrMenu/MenuScrollText.h>

#include <cvrUtil/LocalToWorldVisitor.h>
#include <cvrUtil/Intersection.h>
#include <cvrUtil/OsgMath.h>

#include <cvrKernel/SceneManager.h>
#include <cvrKernel/CVRViewer.h>
#include <cvrKernel/NodeMask.h>

#include <osg/Geometry>

#include <iostream>
#include <sstream>

using namespace cvr;

BoardMenuScrollTextGeometry::BoardMenuScrollTextGeometry() :
        BoardMenuGeometry()
{
    _textLength = 0;
    _lastVisibleRow = 0;
    _lines = 0;
    _scrollActive = false;
    _activeArrow = NO_ARROW;
    _scrollHit = false;
}

BoardMenuScrollTextGeometry::~BoardMenuScrollTextGeometry()
{
}

void BoardMenuScrollTextGeometry::selectItem(bool on)
{
    if(!on)
    {
	if(_scrollHit)
	{
	    osg::Vec4Array* colors = new osg::Vec4Array;
	    colors->push_back(_textColor);
	    _scrollbarGeometry->setColorArray(colors);
	    _scrollHit = false;
	}
	if(_activeArrow == DOWN_ARROW && _downIcon)
	{
	    osg::StateSet * stateset = _downGeode->getOrCreateStateSet();
	    stateset->setTextureAttributeAndModes(0,_downIcon,
		    osg::StateAttribute::ON);
	}
	if(_activeArrow == UP_ARROW && _upIcon)
	{
	    osg::StateSet * stateset = _upGeode->getOrCreateStateSet();
	    stateset->setTextureAttributeAndModes(0,_upIcon,
		    osg::StateAttribute::ON);
	}
	_activeArrow = NO_ARROW;
    }
}

void BoardMenuScrollTextGeometry::createGeometry(MenuItem * item)
{
    _node = new osg::MatrixTransform();
    _intersect = new osg::Geode();
    _node->addChild(_intersect);
    _item = item;

    MenuScrollText * mb = dynamic_cast<MenuScrollText*>(item);
    if(!mb)
    {
        std::cerr << "Error creating BoardMenuScrollTextGeometry."
                << std::endl;
    }

    _textScale = mb->getSizeScale();
    _rows = mb->getRows();

    if(_rows < 2)
    {
	_rows = 2;
    }

    _textWidth = mb->getWidth();
    calcSizes();

    _upIcon = loadIcon("arrow-left.rgb");
    _upIconSelected = loadIcon("arrow-left-highlighted.rgb");
    _downIcon = loadIcon("arrow-right.rgb");
    _downIconSelected = loadIcon("arrow-right-highlighted.rgb");

    parseString(mb->getText());
    prepDisplay(false);
    makeDisplay();

    if(mb->getAppendText().length())
    {
	parseString(mb->getAppendText());
	prepDisplay(true);
	makeDisplay();	
	mb->appendDone();
    }

    _textLength = mb->getLength();

    _text = makeText(_display,_textSize*_textScale,osg::Vec3(0,-2,-_baselineHeight),_textColor,osgText::Text::LEFT_BASE_LINE);
    _textGeode = new osg::Geode();
    _textGeode->addDrawable(_text);
    _node->addChild(_textGeode);

    _height = _maxHeight;
    
    _width = 0;
    if(mb->getIndent())
    {
	_width += _iconHeight + _border;
    }
    _width += _textWidth + _border + _iconHeight;

    osg::Geometry * geo = makeQuad(_iconHeight,-_iconHeight,
            osg::Vec4(1.0,1.0,1.0,1.0),osg::Vec3(_width-_iconHeight,-2,0));

    osg::Vec2Array* texcoords = new osg::Vec2Array;
    texcoords->push_back(osg::Vec2(0,1));
    texcoords->push_back(osg::Vec2(0,0));
    texcoords->push_back(osg::Vec2(1,0));
    texcoords->push_back(osg::Vec2(1,1));
    geo->setTexCoordArray(0,texcoords);

    _upGeode = new osg::Geode();
    _upGeode->addDrawable(geo);
    _node->addChild(_upGeode);

    if(_upIcon)
    {
	osg::StateSet * stateset = _upGeode->getOrCreateStateSet();
        stateset->setTextureAttributeAndModes(0,_upIcon,
                osg::StateAttribute::ON);
    }

    geo = makeQuad(_iconHeight,-_iconHeight,
            osg::Vec4(1.0,1.0,1.0,1.0),osg::Vec3(_width-_iconHeight,-2,-_height+_iconHeight));

    texcoords = new osg::Vec2Array;
    texcoords->push_back(osg::Vec2(0,1));
    texcoords->push_back(osg::Vec2(0,0));
    texcoords->push_back(osg::Vec2(1,0));
    texcoords->push_back(osg::Vec2(1,1));
    geo->setTexCoordArray(0,texcoords);

    _downGeode = new osg::Geode();
    _downGeode->addDrawable(geo);
    _node->addChild(_downGeode);

    _scrollbarGeometry = makeQuad(1.0,1.0,
            _textColor,osg::Vec3(-0.5,-2,-0.5));

    _scrollbarGeode = new osg::Geode();
    _scrollbarGeode->addDrawable(_scrollbarGeometry);
    _scrollbarMT = new osg::MatrixTransform();
    _scrollbarMT->addChild(_scrollbarGeode);
    _node->addChild(_scrollbarMT);

    updateScrollbar();

    if(_downIcon)
    {
	osg::StateSet * stateset = _downGeode->getOrCreateStateSet();
	stateset->setTextureAttributeAndModes(0,_downIcon,
                osg::StateAttribute::ON);
    }

    _scrollDelay = 0.15;
    _currentScrollDelay = _scrollDelay;
    _scrollTimer = 0;
    _timePerScroll = 0.10;
}

void BoardMenuScrollTextGeometry::processEvent(InteractionEvent * event)
{
    static const double slice = 1.0 / 60.0;
    TrackedButtonInteractionEvent * tie = event->asTrackedButtonEvent();
    if(tie)
    {
	switch(event->getInteraction())
	{
	    case BUTTON_DOWN:
	    case BUTTON_DOUBLE_CLICK:
	    {
		if(_activeArrow == UP_ARROW)
		{
		    _lastVisibleRow--;
		    _lastVisibleRow = std::max(_lastVisibleRow,_rows);
		    _lastVisibleRow = std::min(_lastVisibleRow,_lines);
		    makeDisplay();
		    _text->setText(_display);
		    updateScrollbar();
		    _scrollActive = true;
		    _currentScrollDelay = _scrollDelay;
		    _scrollTimer = 0;
		}
		if(_activeArrow == DOWN_ARROW)
		{
		    _lastVisibleRow++;
		    _lastVisibleRow = std::min(_lastVisibleRow,_lines);
		    makeDisplay();
		    _text->setText(_display);
		    updateScrollbar();
		    _scrollActive = true;
		    _currentScrollDelay = _scrollDelay;
		    _scrollTimer = 0;
		}
		break;
	    }
	    case BUTTON_UP:
	    {
		if(_scrollActive)
		{
		    _scrollActive = false;
		}
		break;
	    }
	    case BUTTON_DRAG:
	    {
		if(_activeArrow == UP_ARROW || _activeArrow == DOWN_ARROW)
		{
		    if(_currentScrollDelay > 0.0)
		    {
			//std::cerr << "Scroll Delay" << std::endl;
			_currentScrollDelay -= slice;
			if(_currentScrollDelay < 0.0)
			{
			    _currentScrollDelay = 0.0;
			}
		    }
		    else
		    {
			//std::cerr << "Scroll Timer" << std::endl;
			_scrollTimer += slice;
			int scrollLines = ((int)(_scrollTimer / _timePerScroll));
			if(scrollLines > 0)
			{
			    if(_activeArrow == UP_ARROW)
			    {
				_lastVisibleRow -= scrollLines;
				_lastVisibleRow = std::max(_lastVisibleRow,_rows);
				_lastVisibleRow = std::min(_lastVisibleRow,_lines);
				makeDisplay();
				_text->setText(_display);
				updateScrollbar();
			    }
			    else if(_activeArrow == DOWN_ARROW)
			    {
				_lastVisibleRow += scrollLines;
				_lastVisibleRow = std::min(_lastVisibleRow,_lines);
				makeDisplay();
				_text->setText(_display);
				updateScrollbar();
			    }
			    _scrollTimer -= ((double)scrollLines) * _timePerScroll;
			}
		    }
		}
		else if(_scrollHit)
		{
		    float scrollLines = _lines - _rows;
		    float fullScrollDistance = 1500;
		    if(scrollLines <= 0)
		    {
			break;
		    }

		    osg::Vec3 planepoint(0,0,0);
		    osg::Vec3 planenorm(0,-1,0);

		    osg::Matrix m = osg::Matrix::inverse(getLocalToWorldMatrix(_node));
		    osg::Vec3 lineStart(0,0,0),lineEnd(0,1000,0);
		    lineStart = lineStart * tie->getTransform() * m;
		    lineEnd = lineEnd * tie->getTransform() * m;

		    osg::Vec3 planeIsec;
		    float w;

		    if(linePlaneIntersectionRef(lineStart,lineEnd,planepoint,planenorm,planeIsec,w))
		    {
			float scrollDist = _scrollPoint.z() - planeIsec.z();
			//std::cerr << "Scroll Dist: " << scrollDist << std::endl;
			
			scrollLines = scrollLines * (scrollDist / fullScrollDistance);
			_scrollRows += scrollLines;
			if(((int)_scrollRows) != 0)
			{
			    int rows = (int)_scrollRows;
			    if(rows < 0)
			    {
				_lastVisibleRow += rows;
				_lastVisibleRow = std::max(_lastVisibleRow,_rows);
				_lastVisibleRow = std::min(_lastVisibleRow,_lines);
			    }
			    else
			    {
				_lastVisibleRow += rows;
				_lastVisibleRow = std::min(_lastVisibleRow,_lines);
			    }
			    makeDisplay();
			    _text->setText(_display);
			    updateScrollbar();

			    _scrollRows -= ((float)rows);
			}

			_scrollPoint = planeIsec;
		    }
		}
		break;
	    }
	    default:
		break;
	}
    }
}

void BoardMenuScrollTextGeometry::updateGeometry()
{
    MenuScrollText * mb = dynamic_cast<MenuScrollText*>(_item);

    if(mb->getLength() < _textLength)
    {
	_textLength = 0;
	_lastVisibleRow = 0;
	_words.clear();
	_wordsraw.clear();
	_rowindex.clear();
	_lines = 0;
	// doing this to reset active interactions
	selectItem(false);
	makeDisplay();
	_text->setText(_display);
	updateScrollbar();
    }

    if(mb->getAppendText().length())
    {
	parseString(mb->getAppendText());
	prepDisplay(true);
	makeDisplay();	
	_text->setText(_display);
	mb->appendDone();
	_textLength = mb->getLength();
	updateScrollbar();
    }

    //TODO check if other things have been changed
}

void BoardMenuScrollTextGeometry::update(osg::Vec3 & pointerStart,
        osg::Vec3 & pointerEnd)
{
    if(_scrollActive)
    {
	return;
    }

    std::vector<IsectInfo> isecvec;
    isecvec = getObjectIntersection(SceneManager::instance()->getMenuRoot(),
            pointerStart,pointerEnd);
    for(int i = 0; i < isecvec.size(); i++)
    {
	if(isecvec[i].geode == _upGeode.get())
	{
	    if(_activeArrow != UP_ARROW)
	    {
		if(_activeArrow == DOWN_ARROW && _downIcon)
		{
		    osg::StateSet * stateset = _downGeode->getOrCreateStateSet();
		    stateset->setTextureAttributeAndModes(0,_downIcon,
			    osg::StateAttribute::ON);
		}
		if(_upIconSelected)
		{
		    osg::StateSet * stateset = _upGeode->getOrCreateStateSet();
		    stateset->setTextureAttributeAndModes(0,_upIconSelected,
			    osg::StateAttribute::ON);
		}
		_activeArrow = UP_ARROW;
	    }
	    if(_scrollHit)
	    {
		osg::Vec4Array* colors = new osg::Vec4Array;
		colors->push_back(_textColor);
		_scrollbarGeometry->setColorArray(colors);
		_scrollHit = false;
	    }
	    return;
	}
	else if(isecvec[i].geode == _downGeode.get())
	{
	    if(_activeArrow != DOWN_ARROW)
	    {
		if(_activeArrow == UP_ARROW && _upIcon)
		{
		    osg::StateSet * stateset = _upGeode->getOrCreateStateSet();
		    stateset->setTextureAttributeAndModes(0,_upIcon,
			    osg::StateAttribute::ON);
		}
		if(_downIconSelected)
		{
		    osg::StateSet * stateset = _downGeode->getOrCreateStateSet();
		    stateset->setTextureAttributeAndModes(0,_downIconSelected,
			    osg::StateAttribute::ON);
		}
		_activeArrow = DOWN_ARROW;
	    }
	    if(_scrollHit)
	    {
		osg::Vec4Array* colors = new osg::Vec4Array;
		colors->push_back(_textColor);
		_scrollbarGeometry->setColorArray(colors);
		_scrollHit = false;
	    }
	    return;
	}
    }

    if(_activeArrow == UP_ARROW && _upIcon)
    {
	osg::StateSet * stateset = _upGeode->getOrCreateStateSet();
	stateset->setTextureAttributeAndModes(0,_upIcon,
		osg::StateAttribute::ON);
    }
    if(_activeArrow == DOWN_ARROW && _downIcon)
    {
	osg::StateSet * stateset = _downGeode->getOrCreateStateSet();
	stateset->setTextureAttributeAndModes(0,_downIcon,
		osg::StateAttribute::ON);
    }
    _activeArrow = NO_ARROW;

    osg::Vec3 planepoint(0,0,0);
    osg::Vec3 planenorm(0,-1,0);

    osg::Matrix m = osg::Matrix::inverse(getLocalToWorldMatrix(_node));
    osg::Vec3 lineStart,lineEnd;
    lineStart = pointerStart * m;
    lineEnd = pointerEnd * m;

    osg::Vec3 planeIsec;
    float w;

    if(linePlaneIntersectionRef(lineStart,lineEnd,planepoint,planenorm,planeIsec,w))
    {
	//std::cerr << "Plane Isec x: " << planeIsec.x() << " y: " << planeIsec.y() << " z: " << planeIsec.z() << std::endl;
	if(planeIsec.x() >= _width - _iconHeight && planeIsec.z() < (-_iconHeight) && planeIsec.z() > -(_height - _iconHeight))
	{
	    _scrollPoint = planeIsec;
	    //std::cerr << "Scroll bar hit." << std::endl;
	    if(!_scrollHit)
	    {
		osg::Vec4Array* colors = new osg::Vec4Array;
		colors->push_back(_textColorSelected);
		_scrollbarGeometry->setColorArray(colors);
		
	    }
	    _scrollRows = 0.0;
	    _scrollHit = true;
	}
	else if(_scrollHit)
	{
	    osg::Vec4Array* colors = new osg::Vec4Array;
	    colors->push_back(_textColor);
	    _scrollbarGeometry->setColorArray(colors);
	    _scrollHit = false;
	}

    }
}

void BoardMenuScrollTextGeometry::parseString(std::string & s)
{
    std::string whitespace (" \t\f\v\n\r"); 
    osg::ref_ptr<osgText::Text> slabel = makeText("",_textSize*_textScale,osg::Vec3(0,0,0),osg::Vec4(1.0,1.0,1.0,1.0),osgText::Text::LEFT_TOP);
    size_t ws, index, temp;
    index = 0;

    while(true)
    {
	std::string word;
	float size;
	int leng;
	ws = s.find_first_of(whitespace, index);
	temp = s.find_first_not_of(whitespace, index);
	if(ws == std::string::npos && temp == std::string::npos)
	{
	    break;
	}

	if(ws == index)
	{
	    if(ws == std::string::npos)
	    {
		leng = s.size() - index;
	    }
	    else
	    {
		leng = temp - index;
	    }
	    word = s.substr(index, leng);
	    std::string subword;
	    size_t wsindex, nlindex;
	    int wsleng;
	    wsindex = 0;
	    while(true)
	    {
		nlindex = word.find("\n", wsindex);
		if(nlindex != std::string::npos)
		{
		    wsleng = nlindex - wsindex;
		    if(wsleng > 0)
		    {
			subword = word.substr(wsindex, wsleng);
			slabel->setText(subword);
			osg::BoundingBox bb = slabel->getBound();
			size = bb.xMax() - bb.xMin();
			//size = slabel.getWidth();

			_words.push_back(std::pair<std::string, float>(subword, size));
			_wordsraw.push_back(std::pair<std::string, float>(subword, size));
			wsindex += wsleng;
		    }
		    _words.push_back(std::pair<std::string, float>("\n", 0.0f));
		    _wordsraw.push_back(std::pair<std::string, float>("\n", 0.0f));
		    wsindex++;
		    continue;
		}
		wsleng  = word.size() - wsindex;
		if(wsleng > 0)
		{
		    subword = word.substr(wsindex, wsleng);
		    slabel->setText(subword);
		    osg::BoundingBox bb = slabel->getBound();
		    size = bb.xMax() - bb.xMin();
		    //size = slabel.getWidth();
		    _words.push_back(std::pair<std::string, float>(subword, size));
		    _wordsraw.push_back(std::pair<std::string, float>(subword, size));
		}
		break;
	    }
	    index += leng;
	}
	else
	{
	    if(ws == std::string::npos)
	    {
		leng = s.size() - index;
	    }
	    else
	    {
		leng = ws - index;
	    }
	    word = s.substr(index, leng);

	    slabel->setText(word);
	    osg::BoundingBox bb = slabel->getBound();
	    size = bb.xMax() - bb.xMin();
	    //size = slabel.getWidth();

	    _words.push_back(std::pair<std::string, float>(word, size));
	    _wordsraw.push_back(std::pair<std::string, float>(word, size));

	    index += leng;
	}
    }
}

void BoardMenuScrollTextGeometry::prepDisplay(bool append)
{
    bool atEnd = (_lines == _lastVisibleRow);

    std::string thing1 = "", thing2 = "";

    float rowpos = 0;

    std::list< std::pair< std::string, float > >::iterator it;

    if(!append)
    {
	_lines = 0;
	_words = std::list< std::pair< std::string, float > >(_wordsraw);
	_rowindex.clear();
	it = _words.begin();
	if(it != _words.end())
	{
	    _rowindex.push_back(it);
	    _lines++;
	}
    }
    else
    {
	if(_rowindex.size() > 0)
	{
	    std::vector< std::list< std::pair< std::string, float > >::iterator >::iterator tempit = _rowindex.end();
	    tempit--;
	    it = *tempit;
	}
	else
	{
	    _lines = 0;
	    it = _words.begin();
	    if(it != _words.end())
	    {
		_rowindex.push_back(it);
		_lines++;
	    }
	}
    }

    osg::ref_ptr<osgText::Text> slabel = makeText("",_textSize*_textScale,osg::Vec3(0,0,0),osg::Vec4(1.0,1.0,1.0,1.0),osgText::Text::LEFT_TOP);
    osg::BoundingBox bb;

    while(true)
    {
	if(it == _words.end())
	{
	    break;
	}
	if(it->first != "\n")
	{
	    if(it->second > _textWidth)
	    {
		slabel->setText(thing1);
		bb = slabel->getBound();
		rowpos = bb.xMax() - bb.xMin();
		//rowpos = slabel.getWidth();
		float splitsize = -1000;
		std::string split1, split2;
		int splitindex = 1;
		//cerr << "rowpos: " << rowpos << endl;
		while(splitsize < _textWidth - rowpos)
		{
		    split1 = it->first.substr(0, splitindex);
		    slabel->setText(split1);
		    bb = slabel->getBound();
		    splitsize = bb.xMax() - bb.xMin();
		    //splitsize = slabel.getWidth();
		    //cerr << "splitsize: " << splitsize << endl;
		    splitindex++;
		}
		splitindex = splitindex - 2;
		split1 = it->first.substr(0, splitindex);
		split2 = it->first.substr(splitindex, it->first.size() - splitindex);
		if(split1 == "")
		{
		    //cerr << "moving to next line: " << thing2 << endl;
		    it = _words.insert(it, std::pair<std::string, float>("\n", 0.0f));
		    it++;
		    thing1.clear();
		    thing2.clear();
		    continue;
		}
		else
		{
		    slabel->setText(split2);
		    bb = slabel->getBound();

		    it = _words.insert(it, std::pair<std::string, float>(split2, bb.xMax() - bb.xMin()));
		    it = _words.insert(it, std::pair<std::string, float>(split1, 0.0f));
		    if(rowpos == 0.0)
		    {
			_rowindex.push_back(it);
			_lines++;
		    }
		    it++;
		    it++;
		    it = _words.erase(it);
		    it--;
		    thing1 += split1;
		    thing2 += split1;
		    continue;
		}
	    }
	    thing1 += it->first;
	    slabel->setText(thing1);
	    bb = slabel->getBound();
	    rowpos = bb.xMax() - bb.xMin();
	    //rowpos = slabel.getWidth();
	    if(rowpos < _textWidth)
	    {
		thing2 += it->first;
		it++;
	    }
	    else
	    {
		//cerr << "moving to next line: " << thing2 << endl;
		it = _words.insert(it, std::pair<std::string, float>("\n", 0.0f));
		it++;
		_rowindex.push_back(it);
		thing1 = it->first;
		thing2 = it->first;
		it++;
		_lines++;
		continue;
	    }
	}
	else
	{
	    //it = _words.insert(it, pair<string, float>("\n", 0.0));
	    thing1.clear();
	    thing2.clear();
	    rowpos = 0;
	    it++;
	    if(it != _words.end())
	    {
		_rowindex.push_back(it);
		_lines++;
	    }
	}
    }

    if(atEnd)
    {
	MenuScrollText * mb = dynamic_cast<MenuScrollText*>(_item);
	if(_lastVisibleRow < _rows)
	{
	    _lastVisibleRow = std::min(_rows,_lines);
	}
	if(mb && mb->getFollowEnd())
	{
	    _lastVisibleRow = _lines;
	}
    }
}

void BoardMenuScrollTextGeometry::makeDisplay()
{
    _display = "";
    /*_lines = 0;
      for(std::list< std::pair< std::string, float > >::iterator it = _words.begin(); it != _words.end(); it++)
      {
      _display += it->first;
      if(it->first == "\n")
      {
      _lines++;
      }
      }*/
    //std::cerr << "lines: " << _lines << std::endl;
    /*for(std::vector< std::list< std::pair< std::string, float > >::iterator >::iterator it = _rowindex.begin(); it != _rowindex.end(); it++)
      {
      std::cerr << "first word: " << (*it)->first << std::endl;
      }*/

    if(_lines == 0)
    {
	return;
    }

    std::list< std::pair< std::string, float > >::iterator end;
    if(_lines == _lastVisibleRow)
    {
	end = _words.end();
    }
    else
    {
	end = _rowindex[_lastVisibleRow];
    }

    int firstRow = std::max(_lastVisibleRow-_rows,0);
    //std::cerr << "First row: " << firstRow << std::endl;
    for(std::list< std::pair< std::string, float > >::iterator it = _rowindex[firstRow]; it != end; it++)
    {
	_display += it->first;
    }
}

void BoardMenuScrollTextGeometry::calcSizes()
{
    osg::ref_ptr<osgText::Text> testtext;
    std::stringstream ss;
    for(int i = 0; i < _rows; i++)
    {
	ss << "Ag" << std::endl;
    }

    testtext = makeText(ss.str(),_textSize*_textScale,osg::Vec3(0,0,0),osg::Vec4(1.0,1.0,1.0,1.0),osgText::Text::LEFT_TOP);
    osg::BoundingBox bb = testtext->getBound();
    _maxHeight = bb.zMax() - bb.zMin();

    testtext = makeText("A",_textSize*_textScale,osg::Vec3(0,0,0),osg::Vec4(1.0,1.0,1.0,1.0),osgText::Text::LEFT_TOP);
    bb = testtext->getBound();
    float awidth = bb.xMax() - bb.xMin();
    _baselineHeight = bb.zMax() - bb.zMin();

    testtext = makeText("A A",_textSize*_textScale,osg::Vec3(0,0,0),osg::Vec4(1.0,1.0,1.0,1.0),osgText::Text::LEFT_TOP);
    bb = testtext->getBound();
    float aawidth = bb.xMax() - bb.xMin();

    _spaceSize = aawidth - (2.0*awidth);
}

void BoardMenuScrollTextGeometry::updateScrollbar()
{
    if(_rows <= 2 || _lines <= _rows)
    {
	osg::Matrix m;
	m.makeScale(osg::Vec3(0,0,0));
	_scrollbarMT->setMatrix(m);
    }

    float totalsize = _height - 2.0 * _iconHeight;
    float barsize = totalsize * (((float)_rows) / ((float)_lines));

    osg::Matrix scale,trans;
    scale.makeScale(osg::Vec3(_iconHeight,1.0,barsize));

    float linesAbove = _lastVisibleRow - _rows;
    linesAbove = std::max(linesAbove,0.0f);
    float linesBelow = _lines - _lastVisibleRow;
    float length = linesAbove / (linesAbove + linesBelow);
    length = length * (totalsize - barsize);

    trans.makeTranslate(osg::Vec3(_width-(_iconHeight/2.0),0,-_iconHeight-(barsize/2.0)-length));
    _scrollbarMT->setMatrix(scale*trans);
    
}
