/*
 *  The ManaPlus Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *  Copyright (C) 2011-2016  The ManaPlus Developers
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

#ifndef GUI_WINDOWS_DIDYOUKNOWWINDOW_H
#define GUI_WINDOWS_DIDYOUKNOWWINDOW_H

#include "gui/widgets/linkhandler.h"
#include "gui/widgets/window.h"

#include "listeners/actionlistener.h"

class Button;
class BrowserBox;
class CheckBox;
class ScrollArea;

/**
 * The help dialog.
 */
class DidYouKnowWindow final : public Window,
                               public LinkHandler,
                               public ActionListener
{
    public:
        /**
         * Constructor.
         */
        DidYouKnowWindow();

        A_DELETE_COPY(DidYouKnowWindow)

        void postInit() override final;

        /**
         * Called when receiving actions from the widgets.
         */
        void action(const ActionEvent &event) override final;

        /**
         * Handles link action.
         */
        void handleLink(const std::string &link,
                        MouseEvent *const event) override final;

        void loadData(int num = 0);

        void setVisible(Visible visible) override final;

    private:
        void loadFile(const int num);

        BrowserBox *mBrowserBox A_NONNULLPOINTER;
        ScrollArea *mScrollArea A_NONNULLPOINTER;
        Button *mButtonPrev A_NONNULLPOINTER;
        Button *mButtonNext A_NONNULLPOINTER;
        CheckBox *mOpenAgainCheckBox A_NONNULLPOINTER;
};

extern DidYouKnowWindow *didYouKnowWindow;

#endif  // GUI_WINDOWS_DIDYOUKNOWWINDOW_H
