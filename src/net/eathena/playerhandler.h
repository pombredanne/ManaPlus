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

#ifndef NET_EATHENA_PLAYERHANDLER_H
#define NET_EATHENA_PLAYERHANDLER_H

#include "net/ea/playerhandler.h"

namespace EAthena
{

class PlayerHandler final : public Ea::PlayerHandler
{
    public:
        PlayerHandler();

        A_DELETE_COPY(PlayerHandler)

        void attack(const BeingId id,
                    const Keep keep) const override final;
        void stopAttack() const override final;
        void emote(const uint8_t emoteId) const override final;

        void increaseAttribute(const AttributesT attr,
                               const int amount) const override final;
        void increaseSkill(const uint16_t skillId) const override final;

        void pickUp(const FloorItem *const floorItem) const override final;
        void setDirection(const unsigned char direction) const override final;
        void setDestination(const int x, const int y,
                            const int direction) const override final;
        void changeAction(const BeingActionT &action)
                          const override final;
        void updateStatus(const uint8_t status) const override final;

        void requestOnlineList() const override final;
        void respawn() const override final;
        void setShortcut(const int idx,
                         const uint8_t type,
                         const int id,
                         const int level) const override final;
        void shortcutShiftRow(const int row) const override final;
        void removeOption() const override final;
        void changeCart(const int type) const override final;
        void setMemo() const override final;
        void doriDori() const override final;
        void explosionSpirits() const override final;
        void requestPvpInfo() const override final;
        void revive() const override final;
        void setViewEquipment(const bool allow) const override final;

        void setStat(Net::MessageIn &msg,
                     const int type,
                     const int base,
                     const int mod,
                     const Notify notify) const override final;
};

}  // namespace EAthena

#endif  // NET_EATHENA_PLAYERHANDLER_H
