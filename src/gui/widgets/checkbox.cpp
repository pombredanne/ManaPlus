/*
 *  The ManaPlus Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *  Copyright (C) 2011  The ManaPlus Developers
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

#include "gui/widgets/checkbox.h"

#include "client.h"
#include "configuration.h"
#include "graphics.h"

#include "gui/palette.h"
#include "gui/theme.h"

#include "resources/image.h"

#include <guichan/actionlistener.hpp>

#include "debug.h"

int CheckBox::instances = 0;
float CheckBox::mAlpha = 1.0;
Image *CheckBox::checkBoxNormal;
Image *CheckBox::checkBoxChecked;
Image *CheckBox::checkBoxDisabled;
Image *CheckBox::checkBoxDisabledChecked;
Image *CheckBox::checkBoxNormalHi;
Image *CheckBox::checkBoxCheckedHi;

CheckBox::CheckBox(const std::string &caption, bool selected,
                   gcn::ActionListener* listener, std::string eventId):
    gcn::CheckBox(caption, selected),
    mHasMouse(false)
{
    if (instances == 0)
    {
        Image *checkBox = Theme::getImageFromTheme("checkbox.png");
        if (checkBox)
        {
            checkBoxNormal = checkBox->getSubImage(0, 0, 9, 10);
            checkBoxChecked = checkBox->getSubImage(9, 0, 9, 10);
            checkBoxDisabled = checkBox->getSubImage(18, 0, 9, 10);
            checkBoxDisabledChecked = checkBox->getSubImage(27, 0, 9, 10);
            checkBoxNormalHi = checkBox->getSubImage(36, 0, 9, 10);
            checkBoxCheckedHi = checkBox->getSubImage(45, 0, 9, 10);
            checkBoxNormal->setAlpha(mAlpha);
            checkBoxChecked->setAlpha(mAlpha);
            checkBoxDisabled->setAlpha(mAlpha);
            checkBoxDisabledChecked->setAlpha(mAlpha);
            checkBoxNormalHi->setAlpha(mAlpha);
            checkBoxCheckedHi->setAlpha(mAlpha);
            checkBox->decRef();
        }
        else
        {
            checkBoxNormal = 0;
            checkBoxChecked = 0;
            checkBoxDisabled = 0;
            checkBoxDisabledChecked = 0;
            checkBoxNormalHi = 0;
            checkBoxCheckedHi = 0;
        }
    }

    instances++;

    if (!eventId.empty())
        setActionEventId(eventId);

    if (listener)
        addActionListener(listener);

    setForegroundColor(Theme::getThemeColor(Theme::TEXT));
}

CheckBox::~CheckBox()
{
    instances--;

    if (instances == 0)
    {
        delete checkBoxNormal;
        checkBoxNormal = 0;
        delete checkBoxChecked;
        checkBoxChecked = 0;
        delete checkBoxDisabled;
        checkBoxDisabled = 0;
        delete checkBoxDisabledChecked;
        checkBoxDisabledChecked = 0;
        delete checkBoxNormalHi;
        checkBoxNormalHi = 0;
        delete checkBoxCheckedHi;
        checkBoxCheckedHi = 0;
    }
}

void CheckBox::draw(gcn::Graphics* graphics)
{
    drawBox(graphics);

    graphics->setFont(getFont());
    graphics->setColor(getForegroundColor());

    const int h = getHeight() + getHeight() / 2;

    graphics->drawText(getCaption(), h - 2, 0);
}

void CheckBox::updateAlpha()
{
    float alpha = std::max(Client::getGuiAlpha(),
        Theme::instance()->getMinimumOpacity());

    if (mAlpha != alpha)
    {
        mAlpha = alpha;
        if (checkBoxNormal)
            checkBoxNormal->setAlpha(mAlpha);
        if (checkBoxChecked)
            checkBoxChecked->setAlpha(mAlpha);
        if (checkBoxDisabled)
            checkBoxDisabled->setAlpha(mAlpha);
        if (checkBoxDisabledChecked)
            checkBoxDisabledChecked->setAlpha(mAlpha);
        if (checkBoxNormal)
            checkBoxNormal->setAlpha(mAlpha);
        if (checkBoxCheckedHi)
            checkBoxCheckedHi->setAlpha(mAlpha);
    }
}

void CheckBox::drawBox(gcn::Graphics* graphics)
{
    Image *box;

    if (isEnabled())
    {
        if (isSelected())
        {
            if (mHasMouse)
                box = checkBoxCheckedHi;
            else
                box = checkBoxChecked;
        }
        else
        {
            if (mHasMouse)
                box = checkBoxNormalHi;
            else
                box = checkBoxNormal;
        }
    }
    else
    {
        if (isSelected())
            box = checkBoxDisabledChecked;
        else
            box = checkBoxDisabled;
    }

    updateAlpha();

    if (box)
        static_cast<Graphics*>(graphics)->drawImage(box, 2, 2);
}

void CheckBox::mouseEntered(gcn::MouseEvent& event A_UNUSED)
{
    mHasMouse = true;
}

void CheckBox::mouseExited(gcn::MouseEvent& event A_UNUSED)
{
    mHasMouse = false;
}
