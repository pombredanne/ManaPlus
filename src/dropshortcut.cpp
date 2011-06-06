/*
 *  The Mana World
 *  Copyright (C) 2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  Andrei Karas
 *
 *  This file is part of The Mana World.
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
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "dropshortcut.h"

#include "configuration.h"
#include "inventory.h"
#include "item.h"
#include "localplayer.h"
#include "playerinfo.h"

#include "gui/chatwindow.h"
#include "gui/widgets/chattab.h"

#include "net/inventoryhandler.h"
#include "net/net.h"

#include "utils/stringutils.h"

#include "debug.h"

DropShortcut *dropShortcut;

DropShortcut::DropShortcut():
    mItemSelected(-1),
    mItemColorSelected(1)
{
    for (int i = 0; i < DROP_SHORTCUT_ITEMS; i++)
        mItems[i] = -1;

    load();
    mLastDropIndex = 0;
}

DropShortcut::~DropShortcut()
{
//    save();
}

void DropShortcut::load(bool oldConfig)
{
    Configuration *cfg;
    if (oldConfig)
        cfg = &config;
    else
        cfg = &serverConfig;

    for (int i = 0; i < DROP_SHORTCUT_ITEMS; i++)
    {
        int itemId = static_cast<int>(cfg->getValue("drop" + toString(i), -1));
        int itemColor = static_cast<int>(
            cfg->getValue("dropColor" + toString(i), -1));

        if (itemId != -1)
        {
            mItems[i] = itemId;
            mItemColors[i] = itemColor;
        }
    }
}

void DropShortcut::save()
{
    for (int i = 0; i < DROP_SHORTCUT_ITEMS; i++)
    {
        const int itemId = mItems[i] ? mItems[i] : -1;
        const int itemColor = mItemColors[i] ? mItemColors[i] : 1;
        if (itemId != -1)
        {
            serverConfig.setValue("drop" + toString(i), itemId);
            serverConfig.setValue("dropColor" + toString(i), itemColor);
        }
        else
        {
            serverConfig.deleteKey("drop" + toString(i));
            serverConfig.deleteKey("dropColor" + toString(i));
        }
    }
}

void DropShortcut::dropFirst()
{
    if (!player_node)
        return;

    if (!Client::limitPackets(PACKET_DROP))
        return;

    const int itemId = getItem(0);
    const int itemColor = getItemColor(0);

    if (itemId > 0)
    {
        Item *item = PlayerInfo::getInventory()->findItem(itemId, itemColor);
        if (item && item->getQuantity())
        {
            if (player_node->isServerBuggy())
            {
                Net::getInventoryHandler()->dropItem(item,
                    player_node->getQuickDropCounter());
            }
            else
            {
                for (int i = 0; i < player_node->getQuickDropCounter(); i++)
                    Net::getInventoryHandler()->dropItem(item, 1);
            }
        }
    }
}

void DropShortcut::dropItems(int cnt)
{
    if (!player_node)
        return;

    int n = 0;
    for (int f = 0; f < 9; f++)
    {
        for (int i = 0; i < player_node->getQuickDropCounter(); i++)
        {
            if (!Client::limitPackets(PACKET_DROP))
                return;
            if (dropItem())
                n++;
        }
        if (n >= cnt)
            break;
    }
}

bool DropShortcut::dropItem(int cnt)
{
    int itemId = 0;
    unsigned char itemColor = 1;
    while (mLastDropIndex < DROP_SHORTCUT_ITEMS && itemId < 1)
    {
        itemId = getItem(mLastDropIndex);
        itemColor = getItemColor(mLastDropIndex);
        mLastDropIndex ++;
    }
    if (itemId > 0)
    {
        Item *item = PlayerInfo::getInventory()->findItem(itemId, itemColor);
        if (item && item->getQuantity() > 0)
        {
            Net::getInventoryHandler()->dropItem(item, cnt);
            return true;
        }
    }
    if (mLastDropIndex >= DROP_SHORTCUT_ITEMS)
        mLastDropIndex = 0;

    if (itemId < 1)
    {
        while (mLastDropIndex < DROP_SHORTCUT_ITEMS && itemId < 1)
        {
            itemId = getItem(mLastDropIndex);
            itemColor = getItemColor(mLastDropIndex);
            mLastDropIndex++;
        }
        if (itemId > 0)
        {
            Item *item = PlayerInfo::getInventory()->findItem(
                itemId, itemColor);
            if (item && item->getQuantity() > 0)
            {
                Net::getInventoryHandler()->dropItem(item, cnt);
                return true;
            }
        }
        if (mLastDropIndex >= DROP_SHORTCUT_ITEMS)
            mLastDropIndex = 0;
    }
    return false;
}

void DropShortcut::setItemSelected(Item *item)
{
    if (item)
    {
        mItemSelected = item->getId();
        mItemColorSelected = item->getColor();
    }
    else
    {
        mItemSelected = -1;
        mItemColorSelected = 1;
    }
}

void DropShortcut::setItem(int index)
{
    mItems[index] = mItemSelected;
    mItemColors[index] = mItemColorSelected;
    save();
}
