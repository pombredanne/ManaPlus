/*
 *  The ManaPlus Client
 *  Copyright (C) 2009-2010  The Mana Developers
 *  Copyright (C) 2011-2013  The ManaPlus Developers
 *
 *  This file is part of The ManaPlus Client.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GUI_WIDGETS_VERTCONTAINER_H
#define GUI_WIDGETS_VERTCONTAINER_H

#include "gui/widgets/container.h"

#include <guichan/widgetlistener.hpp>

#include <vector>

#include "localconsts.h"

/**
 * A widget container.
 *
 * This container places it's contents veritcally.
 */
class VertContainer final : public Container, public gcn::WidgetListener
{
    public:
        VertContainer(const Widget2 *const widget,
                      const int verticalItemSize, const bool resizable = true,
                      const int leftSpacing = 0);

        A_DELETE_COPY(VertContainer)

        virtual void add2(gcn::Widget *const widget, const bool resizable,
                          const int spacing = -1);

        virtual void add1(gcn::Widget *const widget, const int spacing = -1);

        virtual void clear();

        void widgetResized(const gcn::Event &event) override;

    private:
        std::vector<gcn::Widget*> mResizableWidgets;
        int mVerticalItemSize;
        int mCount;
        int mNextY;
        int mLeftSpacing;
        int mVerticalSpacing;
        bool mResizable;
};

#endif  // GUI_WIDGETS_VERTCONTAINER_H
