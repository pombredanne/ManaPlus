/*
 *  The ManaPlus Client
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

#include "net/eathena/rouletterecv.h"

#include "logger.h"

#include "net/messagein.h"

#include "debug.h"

namespace EAthena
{

void RouletteRecv::processRouletteInfoAckType(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;
    const int count = (msg.readInt16("len") - 8) / 8;
    msg.readInt32("serial");

    for (int f = 0; f < count; f ++)
    {
        msg.readInt16("row");
        msg.readInt16("position");
        msg.readInt16("item id");
        msg.readInt16("count");
    }
}

void RouletteRecv::processRouletteItemAck(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;
    msg.readUInt8("result");
    msg.readInt16("item id");
}

void RouletteRecv::processRouletteGenerateAckType(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;
    msg.readUInt8("result");
    msg.readInt16("step");
    msg.readInt16("idx");
    msg.readInt16("item id");
    msg.readInt32("remain gold");
    msg.readInt32("remain silver");
    msg.readInt32("remain bronze");
}

void RouletteRecv::processRouletteOpenAck(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;
    msg.readUInt8("result");
    msg.readInt32("serial");
    msg.readUInt8("step");
    msg.readUInt8("idx");
    msg.readInt16("additional item id");
    msg.readInt32("gold point");
    msg.readInt32("silver point");
    msg.readInt32("bronze point");
}

}  // namespace EAthena
