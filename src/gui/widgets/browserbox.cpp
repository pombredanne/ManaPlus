/*
 *  The ManaPlus Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *  Copyright (C) 2011-2016  The ManaPlus Developers
 *  Copyright (C) 2009  Aethyra Development Team
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

#include "gui/widgets/browserbox.h"

#include "itemcolormanager.h"
#include "main.h"
#include "settings.h"

#include "const/resources/item/cards.h"

#include "input/inputmanager.h"

#include "gui/gui.h"
#include "gui/skin.h"

#include "gui/fonts/font.h"

#include "gui/widgets/linkhandler.h"

#include "render/graphics.h"

#include "resources/imageset.h"
#include "resources/iteminfo.h"

#include "resources/db/itemdb.h"

#include "resources/image/image.h"

#include "resources/loaders/imageloader.h"
#include "resources/loaders/imagesetloader.h"

#include "utils/stringutils.h"
#include "utils/timer.h"
#include "utils/translation/podict.h"

#include <algorithm>

#include "debug.h"

namespace
{
    struct MouseOverLink final
    {
        MouseOverLink(const int x, const int y) :
            mX(x),
            mY(y)
        { }

        bool operator() (const BrowserLink &link) const
        {
            return (mX >= link.x1 && mX < link.x2 &&
                    mY >= link.y1 && mY < link.y2);
        }
        const int mX;
        const int mY;
    };
}  // namespace

ImageSet *BrowserBox::mEmotes = nullptr;
int BrowserBox::mInstances = 0;

#define readColor(color) \
    mColors[0][color] = getThemeColor(ThemeColorId::color); \
    mColors[1][color] = getThemeColor(ThemeColorId::color##_OUTLINE)

BrowserBox::BrowserBox(const Widget2 *const widget,
                       const unsigned int mode,
                       const bool opaque,
                       const std::string &skin) :
    Widget(widget),
    MouseListener(),
    mTextRows(),
    mTextRowLinksCount(),
    mLineParts(),
    mLinks(),
    mLinkHandler(nullptr),
    mSkin(nullptr),
    mMode(mode),
    mHighMode(0),
    mSelectedLink(-1),
    mMaxRows(0),
    mHeight(0),
    mWidth(0),
    mYStart(0),
    mUpdateTime(-1),
    mPadding(0),
    mNewLinePadding(15U),
    mItemPadding(0),
    mDataWidth(0),
    mHighlightColor(getThemeColor(ThemeColorId::HIGHLIGHT)),
    mHyperLinkColor(getThemeColor(ThemeColorId::HYPERLINK)),
    mOpaque(opaque),
    mUseLinksAndUserColors(true),
    mUseEmotes(true),
    mAlwaysUpdate(true),
    mProcessVars(false),
    mEnableImages(false),
    mEnableKeys(false),
    mEnableTabs(false)
{
    mAllowLogic = false;

    setFocusable(true);
    addMouseListener(this);

    mBackgroundColor = getThemeColor(ThemeColorId::BACKGROUND);

    if (theme)
        mSkin = theme->load(skin, "browserbox.xml");
    if (mInstances == 0)
    {
        mEmotes = Loader::getImageSet(
            "graphics/sprites/chatemotes.png", 17, 18);
    }
    mInstances ++;

    if (mSkin)
    {
        mPadding = mSkin->getPadding();
        mNewLinePadding = CAST_U32(
            mSkin->getOption("newLinePadding", 15));
        mItemPadding = mSkin->getOption("itemPadding");
        if (mSkin->getOption("highlightBackground"))
            mHighMode |= BACKGROUND;
        if (mSkin->getOption("highlightUnderline"))
            mHighMode |= UNDERLINE;
    }

    readColor(BLACK);
    readColor(RED);
    readColor(GREEN);
    readColor(BLUE);
    readColor(ORANGE);
    readColor(YELLOW);
    readColor(PINK);
    readColor(PURPLE);
    readColor(GRAY);
    readColor(BROWN);

    mForegroundColor = getThemeColor(ThemeColorId::BROWSERBOX);
    mForegroundColor2 = getThemeColor(ThemeColorId::BROWSERBOX_OUTLINE);
}

BrowserBox::~BrowserBox()
{
    if (gui)
        gui->removeDragged(this);

    if (theme)
    {
        theme->unload(mSkin);
        mSkin = nullptr;
    }

    mInstances --;
    if (mInstances == 0)
    {
        if (mEmotes)
        {
            mEmotes->decRef();
            mEmotes = nullptr;
        }
    }
}

void BrowserBox::setLinkHandler(LinkHandler* linkHandler)
{
    mLinkHandler = linkHandler;
}

void BrowserBox::addRow(const std::string &row, const bool atTop)
{
    std::string tmp = row;
    std::string newRow;
    size_t idx1;
    const Font *const font = getFont();
    int linksCount = 0;

    if (getWidth() < 0)
        return;

    if (mProcessVars)
    {
        tmp = replaceAll(tmp, "%VER%", SMALL_VERSION);
        tmp = replaceAll(tmp, "%SUPPORTURL%", settings.supportUrl);
    }

    // Use links and user defined colors
    if (mUseLinksAndUserColors)
    {
        BrowserLink bLink;

        // Check for links in format "@@link|Caption@@"
        const uint32_t sz = CAST_U32(mTextRows.size());

        if (mEnableKeys)
        {
            idx1 = tmp.find("###");
            while (idx1 != std::string::npos)
            {
                const size_t idx2 = tmp.find(";", idx1);
                if (idx2 == std::string::npos)
                    break;

                const std::string str = inputManager.getKeyValueByNameLong(
                    tmp.substr(idx1 + 3, idx2 - idx1 - 3));
                tmp.replace(idx1, idx2 - idx1 + 1, str);

                idx1 = tmp.find("###");
            }
        }

        idx1 = tmp.find("@@");
        while (idx1 != std::string::npos)
        {
            const size_t idx2 = tmp.find("|", idx1);
            const size_t idx3 = tmp.find("@@", idx2);

            if (idx2 == std::string::npos || idx3 == std::string::npos)
                break;
            bLink.link = tmp.substr(idx1 + 2, idx2 - (idx1 + 2));
            bLink.caption = tmp.substr(idx2 + 1, idx3 - (idx2 + 1));
            bLink.y1 = CAST_S32(sz) * font->getHeight();
            bLink.y2 = bLink.y1 + font->getHeight();
            if (bLink.caption.empty())
            {
                bLink.caption = bLink.link;
#ifndef DYECMD
                size_t idx = bLink.link.find(",");
                if (idx != std::string::npos)
                {
                    const int id = atoi(bLink.link.substr(0, idx).c_str());
                    if (id)
                    {
                        std::vector<int> parts;
                        splitToIntVector(parts, bLink.link.substr(idx), ',');
                        while (parts.size() < maxCards)
                            parts.push_back(0);
                        const ItemColor itemColor =
                            ItemColorManager::getColorFromCards(&parts[0]);
                        bLink.caption = ItemDB::get(id).getName(itemColor);
                    }
                }
                else
                {
                    const int id = atoi(bLink.link.c_str());
                    if (id)
                        bLink.caption = ItemDB::get(id).getName();
                }
#endif
                if (translator)
                    bLink.caption = translator->getStr(bLink.caption);
            }

            newRow.append(tmp.substr(0, idx1));

            std::string tmp2 = newRow;
            idx1 = tmp2.find("##");
            while (idx1 != std::string::npos)
            {
                tmp2.erase(idx1, 3);
                idx1 = tmp2.find("##");
            }
            bLink.x1 = font->getWidth(tmp2) - 1;
            bLink.x2 = bLink.x1 + font->getWidth(bLink.caption) + 1;

            if (atTop)
                mLinks.insert(mLinks.begin(), bLink);
            else
                mLinks.push_back(bLink);
            linksCount ++;

            newRow.append("##<").append(bLink.caption);

            tmp.erase(0, idx3 + 2);
            if (!tmp.empty())
                newRow.append("##>");

            idx1 = tmp.find("@@");
        }

        newRow.append(tmp);
    }
    // Don't use links and user defined colors
    else
    {
        newRow = row;
    }

    if (mEnableTabs)
    {
        idx1 = newRow.find("\\t");
        while (idx1 != std::string::npos)
        {
            const size_t idx2 = newRow.find(";", idx1);
            if (idx2 == std::string::npos)
                break;

            const unsigned int newSize = CAST_U32(
                atoi(newRow.substr(
                idx1 + 2, idx2 - idx1 - 2).c_str()));
            std::string str = newRow.substr(0, idx1);
            while (str.size() < newSize)
                str.append(" ");
            str.append(newRow.substr(idx2 + 1));
            newRow = str;
            idx1 = newRow.find("\\t");
        }
    }

    if (atTop)
    {
        mTextRows.push_front(newRow);
        mTextRowLinksCount.push_front(linksCount);
    }
    else
    {
        mTextRows.push_back(newRow);
        mTextRowLinksCount.push_back(linksCount);
    }

    // discard older rows when a row limit has been set
    if (mMaxRows > 0 && !mTextRows.empty())
    {
        while (mTextRows.size() > CAST_SIZE(mMaxRows))
        {
            mTextRows.pop_front();
            int cnt = mTextRowLinksCount.front();
            mTextRowLinksCount.pop_front();

            while (cnt && !mLinks.empty())
            {
                mLinks.erase(mLinks.begin());
                cnt --;
            }
        }
    }

    // Auto size mode
    if (mMode == AUTO_SIZE)
    {
        std::string plain = newRow;
        for (idx1 = plain.find("##");
             idx1 != std::string::npos;
             idx1 = plain.find("##"))
        {
            plain.erase(idx1, 3);
        }

        // Adjust the BrowserBox size
        const int w = font->getWidth(plain);
        if (w > getWidth())
            setWidth(w);
    }

    const int fontHeight = font->getHeight();
    if (mMode == AUTO_WRAP)
    {
        unsigned int y = 0;
        unsigned int nextChar;
        const char *const hyphen = "~";
        const unsigned int hyphenWidth = CAST_U32(
            font->getWidth(hyphen));
        unsigned int x = 0;

        FOR_EACH (TextRowCIter, i, mTextRows)
        {
            std::string tempRow = *i;
            for (uint32_t j = 0, sz = CAST_U32(tempRow.size());
                 j < sz;
                 j++)
            {
                const std::string character = tempRow.substr(j, 1);
                x += CAST_U32(font->getWidth(character));
                nextChar = j + 1;

                // Wraping between words (at blank spaces)
                if (nextChar < sz && tempRow.at(nextChar) == ' ')
                {
                    int nextSpacePos = CAST_U32(
                        tempRow.find(" ", (nextChar + 1)));
                    if (nextSpacePos <= 0)
                        nextSpacePos = CAST_U32(sz) - 1U;

                    const unsigned int nextWordWidth =
                        CAST_U32(font->getWidth(
                        tempRow.substr(nextChar,
                        (CAST_U32(nextSpacePos) - nextChar))));

                    if ((x + nextWordWidth + 10)
                        > CAST_U32(getWidth()))
                    {
                        x = mNewLinePadding;  // Ident in new line
                        y += 1;
                        j ++;
                    }
                }
                // Wrapping looong lines (brutal force)
                else if ((x + 2 * hyphenWidth)
                         > CAST_U32(getWidth()))
                {
                    x = mNewLinePadding;  // Ident in new line
                    y += 1;
                }
            }
        }

        setHeight(fontHeight * (CAST_S32(
            CAST_U32(mTextRows.size()) + y)));
    }
    else
    {
        setHeight(fontHeight * CAST_S32(mTextRows.size()));
    }
    mUpdateTime = 0;
    updateHeight();
}

void BrowserBox::addRow(const std::string &cmd, const char *const text)
{
    addRow(strprintf("@@%s|%s@@", encodeLinkText(cmd).c_str(),
        encodeLinkText(text).c_str()));
}

void BrowserBox::addImage(const std::string &path)
{
    if (!mEnableImages)
        return;

    mTextRows.push_back("~~~" + path);
    mTextRowLinksCount.push_back(0);
}

void BrowserBox::clearRows()
{
    mTextRows.clear();
    mTextRowLinksCount.clear();
    mLinks.clear();
    setWidth(0);
    setHeight(0);
    mSelectedLink = -1;
    mUpdateTime = 0;
    mDataWidth = 0;
    updateHeight();
}

void BrowserBox::mousePressed(MouseEvent &event)
{
    if (!mLinkHandler)
        return;

    const LinkIterator i = std::find_if(mLinks.begin(), mLinks.end(),
        MouseOverLink(event.getX(), event.getY()));

    if (i != mLinks.end())
    {
        mLinkHandler->handleLink(i->link, &event);
        event.consume();
    }
}

void BrowserBox::mouseMoved(MouseEvent &event)
{
    const LinkIterator i = std::find_if(mLinks.begin(), mLinks.end(),
        MouseOverLink(event.getX(), event.getY()));

    mSelectedLink = (i != mLinks.end())
        ? CAST_S32(i - mLinks.begin()) : -1;
}

void BrowserBox::mouseExited(MouseEvent &event A_UNUSED)
{
    mSelectedLink = -1;
}

void BrowserBox::draw(Graphics *const graphics)
{
    BLOCK_START("BrowserBox::draw")
    const ClipRect &cr = graphics->getTopClip();
    mYStart = cr.y - cr.yOffset;
    const int yEnd = mYStart + cr.height;
    if (mYStart < 0)
        mYStart = 0;

    if (mDimension.width != mWidth)
        updateHeight();

    if (mOpaque)
    {
        graphics->setColor(mBackgroundColor);
        graphics->fillRectangle(Rect(0, 0,
            mDimension.width, mDimension.height));
    }

    if (mSelectedLink >= 0 && mSelectedLink
        < CAST_S32(mLinks.size()))
    {
        if ((mHighMode & BACKGROUND))
        {
            BrowserLink &link = mLinks[CAST_SIZE(mSelectedLink)];
            graphics->setColor(mHighlightColor);
            graphics->fillRectangle(Rect(
                link.x1,
                link.y1,
                link.x2 - link.x1,
                link.y2 - link.y1));
        }

        if ((mHighMode & UNDERLINE))
        {
            BrowserLink &link = mLinks[CAST_SIZE(mSelectedLink)];
            graphics->setColor(mHyperLinkColor);
            graphics->drawLine(
                link.x1,
                link.y2,
                link.x2,
                link.y2);
        }
    }

    Font *const font = getFont();

    FOR_EACH (LinePartCIter, i, mLineParts)
    {
        const LinePart &part = *i;
        if (part.mY + 50 < mYStart)
            continue;
        if (part.mY > yEnd)
            break;
        if (!part.mType)
        {
            if (part.mBold)
            {
                boldFont->drawString(graphics,
                    part.mColor,
                    part.mColor2,
                    part.mText,
                    part.mX, part.mY);
            }
            else
            {
                font->drawString(graphics,
                    part.mColor,
                    part.mColor2,
                    part.mText,
                    part.mX, part.mY);
            }
        }
        else if (part.mImage)
        {
            graphics->drawImage(part.mImage, part.mX, part.mY);
        }
    }

    BLOCK_END("BrowserBox::draw")
}

void BrowserBox::safeDraw(Graphics *const graphics)
{
    BrowserBox::draw(graphics);
}

int BrowserBox::calcHeight()
{
    unsigned int y = CAST_U32(mPadding);
    int wrappedLines = 0;
    int moreHeight = 0;
    int maxWidth = mDimension.width - mPadding;
    int link = 0;
    bool bold = false;
    unsigned int wWidth = CAST_U32(maxWidth);

    if (maxWidth < 0)
        return 1;

    const Font *const font = getFont();
    const int fontHeight = font->getHeight() + 2 * mItemPadding;
    const int fontWidthMinus = font->getWidth("-");
    const char *const hyphen = "~";
    const int hyphenWidth = font->getWidth(hyphen);

    Color selColor[2] = {mForegroundColor, mForegroundColor2};
    const Color textColor[2] = {mForegroundColor, mForegroundColor2};
    mLineParts.clear();

    FOR_EACH (TextRowCIter, i, mTextRows)
    {
        unsigned int x = CAST_U32(mPadding);
        const std::string row = *(i);
        bool wrapped = false;
        int objects = 0;

        // Check for separator lines
        if (row.find("---", 0) == 0)
        {
            const int dashWidth = fontWidthMinus;
            for (x = CAST_U32(mPadding); x < wWidth; x ++)
            {
                mLineParts.push_back(LinePart(CAST_S32(x),
                    CAST_S32(y) + mItemPadding,
                    selColor[0], selColor[1], "-", false));
                x += CAST_U32(CAST_S32(
                    dashWidth) - 2);
            }

            y += CAST_U32(fontHeight);
            continue;
        }
        else if (mEnableImages && row.find("~~~", 0) == 0)
        {
            std::string str = row.substr(3);
            const size_t sz = str.size();
            if (sz > 2 && str.substr(sz - 1) == "~")
                str = str.substr(0, sz - 1);
            Image *const img = Loader::getImage(str);
            if (img)
            {
                img->incRef();
                mLineParts.push_back(LinePart(CAST_S32(x),
                    CAST_S32(y) + mItemPadding,
                    selColor[0], selColor[1], img));
                y += CAST_U32(img->getHeight() + 2);
                moreHeight += img->getHeight();
                if (img->getWidth() > maxWidth)
                    maxWidth = img->getWidth() + 2;
            }
            continue;
        }

        Color prevColor[2];
        prevColor[0] = selColor[0];
        prevColor[1] = selColor[1];
        bold = false;

        const int xPadding = CAST_S32(mNewLinePadding) + mPadding;

        for (size_t start = 0, end = std::string::npos;
             start != std::string::npos;
             start = end, end = std::string::npos)
        {
            bool processed(false);

            // Wrapped line continuation shall be indented
            if (wrapped)
            {
                y += CAST_U32(fontHeight);
                x = CAST_U32(xPadding);
                wrapped = false;
            }

            size_t idx1 = end;
            size_t idx2 = end;

            // "Tokenize" the string at control sequences
            if (mUseLinksAndUserColors)
                idx1 = row.find("##", start + 1);
            if (idx1 < idx2)
                end = idx1;
            else
                end = idx2;

            if (start == 0 || mUseLinksAndUserColors)
            {
                // Check for color change in format "##x", x = [L,P,0..9]
                if (row.find("##", start) == start && row.size() > start + 2)
                {
                    const signed char c = row.at(start + 2);

                    bool valid(false);
                    const Color col[2] =
                    {
                        getThemeCharColor(c, valid),
                        getThemeCharColor(CAST_S8(
                            c | 0x80), valid)
                    };

                    if (c == '>')
                    {
                        selColor[0] = prevColor[0];
                        selColor[1] = prevColor[1];
                    }
                    else if (c == '<')
                    {
                        prevColor[0] = selColor[0];
                        prevColor[1] = selColor[1];
                        selColor[0] = col[0];
                        selColor[1] = col[1];
                    }
                    else if (c == 'B')
                    {
                        bold = true;
                    }
                    else if (c == 'b')
                    {
                        bold = false;
                    }
                    else if (valid)
                    {
                        selColor[0] = col[0];
                        selColor[1] = col[1];
                    }
                    else
                    {
                        switch (c)
                        {
                            case '0':
                                selColor[0] = mColors[0][BLACK];
                                selColor[1] = mColors[1][BLACK];
                                break;
                            case '1':
                                selColor[0] = mColors[0][RED];
                                selColor[1] = mColors[1][RED];
                                break;
                            case '2':
                                selColor[0] = mColors[0][GREEN];
                                selColor[1] = mColors[1][GREEN];
                                break;
                            case '3':
                                selColor[0] = mColors[0][BLUE];
                                selColor[1] = mColors[1][BLUE];
                                break;
                            case '4':
                                selColor[0] = mColors[0][ORANGE];
                                selColor[1] = mColors[1][ORANGE];
                                break;
                            case '5':
                                selColor[0] = mColors[0][YELLOW];
                                selColor[1] = mColors[1][YELLOW];
                                break;
                            case '6':
                                selColor[0] = mColors[0][PINK];
                                selColor[1] = mColors[1][PINK];
                                break;
                            case '7':
                                selColor[0] = mColors[0][PURPLE];
                                selColor[1] = mColors[1][PURPLE];
                                break;
                            case '8':
                                selColor[0] = mColors[0][GRAY];
                                selColor[1] = mColors[1][GRAY];
                                break;
                            case '9':
                                selColor[0] = mColors[0][BROWN];
                                selColor[1] = mColors[1][BROWN];
                                break;
                            default:
                                selColor[0] = textColor[0];
                                selColor[1] = textColor[1];
                                break;
                        }
                    }

                    if (c == '<' && link < CAST_S32(mLinks.size()))
                    {
                        const int size = font->getWidth(
                            mLinks[CAST_SIZE(link)].caption) + 1;

                        BrowserLink &linkRef = mLinks[CAST_SIZE(
                            link)];
                        linkRef.x1 = CAST_S32(x);
                        linkRef.y1 = CAST_S32(y);
                        linkRef.x2 = linkRef.x1 + size;
                        linkRef.y2 = CAST_S32(y) + fontHeight - 1;
                        link++;
                    }

                    processed = true;
                    start += 3;
                    if (start == row.size())
                        break;
                }
            }
            if (mUseEmotes)
                idx2 = row.find("%%", start + 1);
            if (idx1 < idx2)
                end = idx1;
            else
                end = idx2;
            if (mUseEmotes)
            {
                // check for emote icons
                if (row.size() > start + 2 && row.substr(start, 2) == "%%")
                {
                    if (objects < 5)
                    {
                        const int cid = row.at(start + 2) - '0';
                        if (cid >= 0)
                        {
                            if (mEmotes)
                            {
                                const size_t sz = mEmotes->size();
                                if (CAST_SIZE(cid) < sz)
                                {
                                    Image *const img = mEmotes->get(
                                        CAST_SIZE(cid));
                                    if (img)
                                    {
                                        mLineParts.push_back(LinePart(
                                            CAST_S32(x),
                                            CAST_S32(y) + mItemPadding,
                                            selColor[0], selColor[1], img));
                                        x += 18;
                                    }
                                }
                            }
                        }
                        objects ++;
                        processed = true;
                    }

                    start += 3;
                    if (start == row.size())
                    {
                        if (x > mDataWidth)
                            mDataWidth = x;
                        break;
                    }
                }
            }
            const size_t len = (end == std::string::npos) ? end : end - start;

            if (start >= row.length())
                break;

            std::string part = row.substr(start, len);
            int width = 0;
            if (bold)
                width = boldFont->getWidth(part);
            else
                width = font->getWidth(part);

            // Auto wrap mode
            if (mMode == AUTO_WRAP && wWidth > 0 && width > 0
                && (x + CAST_U32(width) + 10) > wWidth)
            {
                bool forced = false;

                /* FIXME: This code layout makes it easy to crash remote
                   clients by talking garbage. Forged long utf-8 characters
                   will cause either a buffer underflow in substr or an
                   infinite loop in the main loop. */
                do
                {
                    if (!forced)
                        end = row.rfind(' ', end);

                    // Check if we have to (stupidly) force-wrap
                    if (end == std::string::npos || end <= start)
                    {
                        forced = true;
                        end = row.size();
                        x += CAST_U32(hyphenWidth);
                        continue;
                    }

                    // Skip to the start of the current character
                    while ((row[end] & 192) == 128)
                        end--;
                    end--;  // And then to the last byte of the previous one

                    part = row.substr(start, end - start + 1);
                    if (bold)
                        width = boldFont->getWidth(part);
                    else
                        width = font->getWidth(part);
                }
                while (end > start &&
                       width > 0 &&
                       (x + CAST_U32(width) + 10) > wWidth);

                if (forced)
                {
                    x -= CAST_U32(hyphenWidth);
                    mLineParts.push_back(LinePart(
                        CAST_S32(wWidth) - hyphenWidth,
                        CAST_S32(y) + mItemPadding,
                        selColor[0], selColor[1], hyphen, bold));
                    end++;  // Skip to the next character
                }
                else
                {
                    end += 2;  // Skip to after the space
                }

                wrapped = true;
                wrappedLines++;
            }

            mLineParts.push_back(LinePart(CAST_S32(x),
                CAST_S32(y) + mItemPadding,
                selColor[0], selColor[1], part.c_str(), bold));

            if (bold)
                width = boldFont->getWidth(part);
            else
                width = font->getWidth(part);

            if (mMode == AUTO_WRAP && (width == 0 && !processed))
                break;

            x += CAST_U32(width);
            if (x > mDataWidth)
                mDataWidth = x;
        }
        y += CAST_U32(fontHeight);
    }
    if (CAST_S32(wWidth) != maxWidth)
        setWidth(maxWidth);

    return (CAST_S32(mTextRows.size()) + wrappedLines)
        * fontHeight + moreHeight + 2 * mPadding;
}

void BrowserBox::updateHeight()
{
    if (mAlwaysUpdate || mUpdateTime != cur_time
        || mTextRows.size() < 3 || !mUpdateTime)
    {
        mWidth = mDimension.width;
        mHeight = calcHeight();
        setHeight(mHeight);
        mUpdateTime = cur_time;
    }
}

std::string BrowserBox::getTextAtPos(const int x, const int y) const
{
    int textX = 0;
    int textY = 0;

    getAbsolutePosition(textX, textY);
    if (x < textX || y < textY)
        return std::string();

    textY = y - textY;
    std::string str;
    int lastY = 0;

    FOR_EACH (LinePartCIter, i, mLineParts)
    {
        const LinePart &part = *i;
        if (part.mY + 50 < mYStart)
            continue;
        if (part.mY > textY)
            break;

        if (part.mY > lastY)
        {
            str = part.mText;
            lastY = part.mY;
        }
        else
        {
            str.append(part.mText);
        }
    }

    return str;
}

void BrowserBox::setForegroundColorAll(const Color &color1,
                                       const Color &color2)
{
    mForegroundColor = color1;
    mForegroundColor2 = color2;
}

void BrowserBox::moveSelectionUp()
{
    if (mSelectedLink <= 0)
        mSelectedLink = CAST_S32(mLinks.size()) - 1;
    else
        mSelectedLink --;
}

void BrowserBox::moveSelectionDown()
{
    mSelectedLink ++;
    if (mSelectedLink >= static_cast<signed int>(mLinks.size()))
        mSelectedLink = 0;
}

void BrowserBox::selectSelection()
{
    if (!mLinkHandler ||
        mSelectedLink < 0 ||
        mSelectedLink >= static_cast<signed int>(mLinks.size()))
    {
        return;
    }

    mLinkHandler->handleLink(mLinks[CAST_SIZE(mSelectedLink)].link,
        nullptr);
}
