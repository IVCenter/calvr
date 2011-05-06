#ifndef _CUI_CUI_H_
#define _CUI_CUI_H_

// OSG:
#include "Widget.h"
#include <osg/Node>

/** This class contains global variables and functions for CUI. 
  It is also the documentation central.
*/

namespace cui 
{

class CUIEXPORT CUI
{
  public:
    enum DisplayType    ///< type of (virtual) environment rendered in
    {
      CAVE, FISHTANK, DESKTOP
    };
    static DisplayType _display;

    static osg::Matrix computeLocal2Root(const osg::Node*);
    static bool isChild(osg::Node*, osg::Node*);
};

}

#endif
