#include <cvrMenu/MenuCollection.h>
#include <cvrMenu/MenuSystem.h>

using namespace cvr;

MenuCollection::MenuCollection() :
        MenuItem()
{
}

MenuCollection::~MenuCollection()
{
    for(int i = 0; i < _children.size(); ++i)
    {
	_children[i]->setParent(NULL);
    }
    _children.clear();
}

bool MenuCollection::isDirty()
{
    if(_dirty)
    {
	return true;
    }

    bool dirty = false;
    for(int i = 0; i < _children.size(); ++i)
    {
	if(_children[i]->isDirty())
	{
	    dirty = true;
	}
    }
    _dirty = dirty;
    return _dirty;
}

void MenuCollection::addItem(MenuItem * item)
{
    if(!item)
    {
        return;
    }

    for(int i = 0; i < _children.size(); i++)
    {
        if(_children[i] == item)
        {
            return;
        }
    }
    _children.push_back(item);
    _dirty = true;
    item->setParent(this);
}

void MenuCollection::addItem(MenuItem * item, int position)
{
    if(!item)
    {
        return;
    }

    if(getItemPosition(item) >= 0)
    {
        return;
    }

    if(position < 0 || position >= _children.size())
    {
        _children.push_back(item);
    }
    else
    {
        std::vector<MenuItem*>::iterator it = _children.begin();
        it += position;
        _children.insert(it,item);
    }

    _dirty = true;
    item->setParent(this);
}

void MenuCollection::removeItem(MenuItem * item)
{
    for(std::vector<MenuItem*>::iterator it = _children.begin();
            it != _children.end(); it++)
    {
        if((*it) == item)
        {
            _children.erase(it);
            _dirty = true;
            item->setParent(0);
            return;
        }
    }
}

int MenuCollection::getItemPosition(MenuItem * item)
{
    for(int i = 0; i < _children.size(); i++)
    {
        if(_children[i] == item)
        {
            return i;
        }
    }
    return -1;
}

MenuItem * MenuCollection::getChild(int i)
{
    if(i < 0 || i >= _children.size())
    {
        return NULL;
    }
    return _children[i];
}

std::vector<MenuItem*> & MenuCollection::getChildren()
{
    return _children;
}

int MenuCollection::getNumChildren()
{
    return _children.size();
}
