#ifndef CALVR_MENU_ITEM_GROUP_H
#define CALVR_MENU_ITEM_GROUP_H

#include <cvrMenu/Export.h>
#include <cvrMenu/MenuCollection.h>

namespace cvr
{

class CVRMENU_EXPORT MenuItemGroup : public MenuCollection
{
    public:
        enum LayoutHint
        {
            ROW_LAYOUT=0,
            COL_LAYOUT
        };

        enum AlignmentHint
        {
            ALIGN_LEFT_INDENT=0,
            ALIGN_LEFT,
            ALIGN_CENTER
        };

        MenuItemGroup(LayoutHint layoutHint = ROW_LAYOUT, AlignmentHint alignHint = ALIGN_CENTER);
        virtual ~MenuItemGroup();

        virtual MenuItemType getType()
        {
            return ITEM_GROUP;
        }

        void setLayoutHint(LayoutHint hint)
        {
            _layoutHint = hint;
        }

        void setAlignmentHint(AlignmentHint hint)
        {
            _alignHint = hint;
        }

        LayoutHint getLayoutHint()
        {
            return _layoutHint;
        }

        AlignmentHint getAlignmentHint()
        {
            return _alignHint;
        }

    protected:
        LayoutHint _layoutHint;
        AlignmentHint _alignHint;
};

}

#endif
