/*
 *  The ManaPlus Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *  Copyright (C) 2011-2014  The ManaPlus Developers
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

#include "net/eathena/npchandler.h"

#include "being/localplayer.h"

#include "gui/windows/npcdialog.h"

#include "net/eathena/messageout.h"
#include "net/eathena/protocol.h"

#include "net/ea/eaprotocol.h"

#include "debug.h"

extern Net::NpcHandler *npcHandler;

namespace EAthena
{

NpcHandler::NpcHandler() :
    MessageHandler(),
    Ea::NpcHandler()
{
    static const uint16_t _messages[] =
    {
        SMSG_NPC_CHOICE,
        SMSG_NPC_MESSAGE,
        SMSG_NPC_NEXT,
        SMSG_NPC_CLOSE,
        SMSG_NPC_INT_INPUT,
        SMSG_NPC_STR_INPUT,
        SMSG_NPC_CUTIN,
        SMSG_NPC_VIEWPOINT,
        SMSG_NPC_SHOW_PROGRESS_BAR,
        0
    };
    handledMessages = _messages;
    npcHandler = this;
}

void NpcHandler::handleMessage(Net::MessageIn &msg)
{
    switch (msg.getId())
    {
        case SMSG_NPC_CHOICE:
            processNpcChoice(msg);
            break;

        case SMSG_NPC_MESSAGE:
            processNpcMessage(msg);
            break;

        case SMSG_NPC_CLOSE:
            processNpcClose(msg);
            break;

        case SMSG_NPC_NEXT:
            processNpcNext(msg);
            break;

        case SMSG_NPC_INT_INPUT:
            processNpcIntInput(msg);
            break;

        case SMSG_NPC_STR_INPUT:
            processNpcStrInput(msg);
            break;

        case SMSG_NPC_CUTIN:
            processNpcCutin(msg);
            break;

        case SMSG_NPC_VIEWPOINT:
            processNpcViewPoint(msg);
            break;

        case SMSG_NPC_SHOW_PROGRESS_BAR:
            processNpcShowProgressBar(msg);
            break;

        default:
            break;
    }

    mDialog = nullptr;
}

void NpcHandler::talk(const int npcId) const
{
    MessageOut outMsg(CMSG_NPC_TALK);
    outMsg.writeInt32(npcId);
    outMsg.writeInt8(0);  // unused
}

void NpcHandler::nextDialog(const int npcId) const
{
    MessageOut outMsg(CMSG_NPC_NEXT_REQUEST);
    outMsg.writeInt32(npcId);
}

void NpcHandler::closeDialog(const int npcId)
{
    MessageOut outMsg(CMSG_NPC_CLOSE);
    outMsg.writeInt32(npcId);

    const NpcDialogs::iterator it = NpcDialog::mNpcDialogs.find(npcId);
    if (it != NpcDialog::mNpcDialogs.end())
    {
        NpcDialog *const dialog = (*it).second;
        if (dialog)
            dialog->close();
        if (dialog == mDialog)
            mDialog = nullptr;
        NpcDialog::mNpcDialogs.erase(it);
    }
}

void NpcHandler::listInput(const int npcId, const unsigned char value) const
{
    MessageOut outMsg(CMSG_NPC_LIST_CHOICE);
    outMsg.writeInt32(npcId);
    outMsg.writeInt8(value);
}

void NpcHandler::integerInput(const int npcId, const int value) const
{
    MessageOut outMsg(CMSG_NPC_INT_RESPONSE);
    outMsg.writeInt32(npcId);
    outMsg.writeInt32(value);
}

void NpcHandler::stringInput(const int npcId, const std::string &value) const
{
    MessageOut outMsg(CMSG_NPC_STR_RESPONSE);
    outMsg.writeInt16(static_cast<int16_t>(value.length() + 9));
    outMsg.writeInt32(npcId);
    outMsg.writeString(value, static_cast<int>(value.length()));
    outMsg.writeInt8(0);  // Prevent problems with string reading
}

void NpcHandler::buy(const int beingId) const
{
    MessageOut outMsg(CMSG_NPC_BUY_SELL_REQUEST);
    outMsg.writeInt32(beingId);
    outMsg.writeInt8(0);  // Buy
}

void NpcHandler::sell(const int beingId) const
{
    MessageOut outMsg(CMSG_NPC_BUY_SELL_REQUEST);
    outMsg.writeInt32(beingId);
    outMsg.writeInt8(1);  // Sell
}

void NpcHandler::buyItem(const int beingId A_UNUSED, const int itemId,
                         const unsigned char color A_UNUSED,
                         const int amount) const
{
    MessageOut outMsg(CMSG_NPC_BUY_REQUEST);
    outMsg.writeInt16(8);  // One item (length of packet)
    outMsg.writeInt16(static_cast<int16_t>(amount));
    outMsg.writeInt16(static_cast<int16_t>(itemId));
}

void NpcHandler::sellItem(const int beingId A_UNUSED,
                          const int itemId, const int amount) const
{
    MessageOut outMsg(CMSG_NPC_SELL_REQUEST);
    outMsg.writeInt16(8);  // One item (length of packet)
    outMsg.writeInt16(static_cast<int16_t>(itemId + INVENTORY_OFFSET));
    outMsg.writeInt16(static_cast<int16_t>(amount));
}

void NpcHandler::completeProgressBar() const
{
    MessageOut outMsg(CMSG_NPC_COMPLETE_PROGRESS_BAR);
}

void NpcHandler::produceMix(const int nameId,
                            const int materialId1,
                            const int materialId2,
                            const int materialId3) const
{
    MessageOut outMsg(CMSG_NPC_PRODUCE_MIX);
    outMsg.writeInt16(nameId, "name id");
    outMsg.writeInt16(materialId1, "material 1");
    outMsg.writeInt16(materialId2, "material 2");
    outMsg.writeInt16(materialId3, "material 3");
}

void NpcHandler::cooking(const CookingType::Type type,
                         const int nameId) const
{
    MessageOut outMsg(CMSG_NPC_COOKING);
    outMsg.writeInt16(static_cast<int16_t>(type), "type");
    outMsg.writeInt16(nameId, "name id");
}

void NpcHandler::repair(const int index) const
{
    MessageOut outMsg(CMSG_NPC_REPAIR);
    outMsg.writeInt16(static_cast<int16_t>(index), "index");
}

int NpcHandler::getNpc(Net::MessageIn &msg)
{
    if (msg.getId() == SMSG_NPC_CHOICE
        || msg.getId() == SMSG_NPC_MESSAGE)
    {
        msg.readInt16("len");
    }

    const int npcId = msg.readInt32("npc id");

    const NpcDialogs::const_iterator diag = NpcDialog::mNpcDialogs.find(npcId);
    mDialog = nullptr;

    if (msg.getId() == SMSG_NPC_VIEWPOINT)
        return npcId;

    if (diag == NpcDialog::mNpcDialogs.end())
    {
        // Empty dialogs don't help
        if (msg.getId() == SMSG_NPC_CLOSE)
        {
            closeDialog(npcId);
            return npcId;
        }
        else if (msg.getId() == SMSG_NPC_NEXT)
        {
            nextDialog(npcId);
            return npcId;
        }
        else
        {
            mDialog = new NpcDialog(npcId);
            mDialog->postInit();
            mDialog->saveCamera();
            if (localPlayer)
                localPlayer->stopWalking(false);
            NpcDialog::mNpcDialogs[npcId] = mDialog;
        }
    }
    else
    {
        NpcDialog *const dialog = diag->second;
        if (mDialog && mDialog != dialog)
            mDialog->restoreCamera();
        mDialog = dialog;
        if (mDialog)
            mDialog->saveCamera();
    }
    return npcId;
}

void NpcHandler::processNpcCutin(Net::MessageIn &msg)
{
    msg.readString(64);  // image name
    msg.readUInt8();     // type
}

void NpcHandler::processNpcViewPoint(Net::MessageIn &msg)
{
    // +++ probably need add nav point and start moving to it
    msg.readInt32("npc id");
    msg.readInt32("type");  // 0 display for 15 sec,
                            // 1 display until teleport,
                            // 2 remove
    msg.readInt32("x");
    msg.readInt32("y");
    msg.readUInt8("number");  // can be used for scripts
    msg.readInt32("color");
}

void NpcHandler::processNpcShowProgressBar(Net::MessageIn &msg) const
{
    // +++ probably need show progress bar in npc dialog
    msg.readInt32("color");
    msg.readInt32("seconds");
}

}  // namespace EAthena
