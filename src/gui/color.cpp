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

/*      _______   __   __   __   ______   __   __   _______   __   __
 *     / _____/\ / /\ / /\ / /\ / ____/\ / /\ / /\ / ___  /\ /  |\/ /\
 *    / /\____\// / // / // / // /\___\// /_// / // /\_/ / // , |/ / /
 *   / / /__   / / // / // / // / /    / ___  / // ___  / // /| ' / /
 *  / /_// /\ / /_// / // / // /_/_   / / // / // /\_/ / // / |  / /
 * /______/ //______/ //_/ //_____/\ /_/ //_/ //_/ //_/ //_/ /|_/ /
 * \______\/ \______\/ \_\/ \_____\/ \_\/ \_\/ \_\/ \_\/ \_\/ \_\/
 *
 * Copyright (c) 2004 - 2008 Olof Naessén and Per Larsson
 *
 *
 * Per Larsson a.k.a finalman
 * Olof Naessén a.k.a jansem/yakslem
 *
 * Visit: http://guichan.sourceforge.net
 *
 * License: (BSD)
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of Guichan nor the names of its contributors may
 *    be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 * TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * For comments regarding functions please see the header file.
 */

#include "gui/color.h"

#include "debug.h"

Color::Color() :
    r(0U),
    g(0U),
    b(0U),
    a(255U)
{
}

Color::Color(const unsigned int color) :
    r((color >> 16) & 0xFFU),
    g((color >>  8) & 0xFFU),
    b(color         & 0xFFU),
    a(255U)
{
}

Color::Color(const unsigned int ar,
             const unsigned int ag,
             const unsigned int ab,
             const unsigned int aa) :
    r(ar),
    g(ag),
    b(ab),
    a(aa)
{
}

Color Color::operator+(const Color& color) const
{
    Color result(r + color.r,
                  g + color.g,
                  b + color.b,
                  255U);

    result.r = (result.r > 255U ? 255U : result.r);
    result.g = (result.g > 255U ? 255U : result.g);
    result.b = (result.b > 255U ? 255U : result.b);

    return result;
}

Color Color::operator-(const Color& color) const
{
    Color result(r - color.r,
                 g - color.g,
                 b - color.b,
                 255U);

    result.r = (result.r > 255U ? 255U : result.r);
    result.g = (result.g > 255U ? 255U : result.g);
    result.b = (result.b > 255U ? 255U : result.b);

    return result;
}

Color Color::operator*(const float value) const
{
    Color result(CAST_U32(static_cast<float>(r) * value),
                 CAST_U32(static_cast<float>(g) * value),
                 CAST_U32(static_cast<float>(b) * value),
                 a);

    result.r = (result.r > 255U ? 255U : result.r);
    result.g = (result.g > 255U ? 255U : result.g);
    result.b = (result.b > 255U ? 255U : result.b);

    return result;
}

bool Color::operator==(const Color& color) const
{
    return r == color.r && g == color.g && b == color.b && a == color.a;
}

bool Color::operator!=(const Color& color) const
{
    return !(r == color.r && g == color.g && b == color.b && a == color.a);
}
