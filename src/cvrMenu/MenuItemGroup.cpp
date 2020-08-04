#include <cvrMenu/MenuItemGroup.h>

using namespace cvr;

MenuItemGroup::MenuItemGroup(LayoutHint layoutHint, AlignmentHint alignHint) : MenuCollection()
{
    _layoutHint = layoutHint;
    _alignHint = alignHint;
}

MenuItemGroup::~MenuItemGroup()
{
}
