/*
 *  The ManaPlus Client
 *  Copyright (C) 2012-2016  The ManaPlus Developers
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

#include "eventsmanager.h"

#include "configuration.h"
#include "client.h"
#ifndef DYECMD
#include "game.h"
#endif
#ifdef USE_MUMBLE
#include "mumblemanager.h"
#endif
#include "sdlshared.h"
#include "settings.h"

#include "gui/windowmanager.h"

#include "being/localplayer.h"

#include "input/inputmanager.h"

#ifdef USE_SDL2
#include "render/graphics.h"
#else  // USE_SDL2
#include "logger.h"
#endif  // USE_SDL2

#include "utils/process.h"

#include "debug.h"

EventsManager eventsManager;

EventsManager::EventsManager() :
    mLogInput(false)
{
}

EventsManager::~EventsManager()
{
    CHECKLISTENERS
}

void EventsManager::init()
{
    mLogInput = config.getBoolValue("logInput");
    config.addListener("logInput", this);
}

void EventsManager::shutdown()
{
    config.removeListeners(this);
}

bool EventsManager::handleCommonEvents(const SDL_Event &event) const
{
    BLOCK_START("EventsManager::handleCommonEvents")
    if (mLogInput)
        logEvent(event);

    switch (event.type)
    {
        case SDL_QUIT:
            client->setState(State::EXIT);
            logger->log1("force exit");
            BLOCK_END("EventsManager::handleCommonEvents")
            return true;
#ifdef USE_SDL2
        case SDL_WINDOWEVENT:
            handleSDL2WindowEvent(event);
            BLOCK_END("EventsManager::handleCommonEvents")
            return true;
#else
        case SDL_VIDEORESIZE:
            WindowManager::resizeVideo(event.resize.w,
                event.resize.h,
                false);
            BLOCK_END("EventsManager::handleCommonEvents")
            return true;
        case SDL_ACTIVEEVENT:
            handleActive(event);
            BLOCK_END("EventsManager::handleCommonEvents")
            return true;
#ifdef ANDROID
        case SDL_KEYBOARDSHOW:
            WindowManager::updateScreenKeyboard(event.user.code);
            BLOCK_END("EventsManager::handleCommonEvents")
            return true;
        case SDL_ACCELEROMETER:
            break;
#endif
#endif
        default:
            break;
    }

    if (inputManager.handleEvent(event))
    {
        BLOCK_END("EventsManager::handleCommonEvents")
        return true;
    }

    BLOCK_END("EventsManager::handleCommonEvents")
    return false;
}

bool EventsManager::handleEvents() const
{
    BLOCK_START("EventsManager::handleEvents")
#ifndef DYECMD
    if (Game::instance())
    {
        // Let the game handle the events while it is active
        Game::instance()->handleInput();
    }
    else
#endif
    {
        SDL_Event event;
        // Handle SDL events
        while (SDL_WaitEventTimeout(&event, 0))
        {
            if (!handleCommonEvents(event))
            {
                switch (event.type)
                {
#ifdef ANDROID
#ifndef USE_SDL2
                    case SDL_ACTIVEEVENT:
                        if ((event.active.state & SDL_APPACTIVE)
                            && !event.active.gain)
                        {
                            client->setState(State::EXIT);
                            logger->log1("exit on lost focus");
                        }
                        break;

#endif
#else
#ifndef USE_SDL2
#endif
#endif
                    default:
                        break;
                }
            }

#ifdef USE_MUMBLE
            if (localPlayer && mumbleManager)
            {
                mumbleManager->setPos(localPlayer->getTileX(),
                    localPlayer->getTileY(), localPlayer->getDirection());
            }
#endif
        }
        if (client->getState() == State::EXIT)
        {
            BLOCK_END("EventsManager::handleEvents")
            return true;
        }
    }
    BLOCK_END("EventsManager::handleEvents")
    return false;
}

void EventsManager::handleGameEvents() const
{
    BLOCK_START("EventsManager::handleGameEvents")
#ifndef DYECMD
    Game *const game = Game::instance();

    // Events
    SDL_Event event;
    while (SDL_WaitEventTimeout(&event, 0))
    {
        if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
            game->updateHistory(event);
        game->checkKeys();

        if (handleCommonEvents(event))
            break;
    }  // End while
#endif
    BLOCK_END("EventsManager::handleGameEvents")
}

void EventsManager::optionChanged(const std::string &name)
{
    if (name == "logInput")
        mLogInput = config.getBoolValue("logInput");
}

void EventsManager::logEvent(const SDL_Event &event)
{
    switch (event.type)
    {
#ifdef USE_SDL2
#define winEventLog(name, name2) \
    case name: \
        str = name2; \
        break

        case SDL_MOUSEMOTION:
            logger->log("event: SDL_MOUSEMOTION: %u,%d,%d",
                event.motion.state,
                CAST_S32(event.motion.x),
                CAST_S32(event.motion.y));
            break;
        case SDL_FINGERDOWN:
        {
            const SDL_TouchFingerEvent &touch = event.tfinger;
            const int w = mainGraphics->mWidth;
            const int h = mainGraphics->mHeight;
            logger->log("event: SDL_FINGERDOWN: %u,%u (%f,%f) (%f,%f)",
                CAST_U32(touch.touchId),
                CAST_U32(touch.fingerId),
                touch.x * w, touch.y * w,
                touch.dx * w, touch.dy * h);
            break;
        }
        case SDL_FINGERUP:
        {
            const SDL_TouchFingerEvent &touch = event.tfinger;
            const int w = mainGraphics->mWidth;
            const int h = mainGraphics->mHeight;
            logger->log("event: SDL_FINGERUP: %u,%u (%f,%f) (%f,%f)",
                CAST_U32(touch.touchId),
                CAST_U32(touch.fingerId),
                touch.x * w, touch.y * w,
                touch.dx * w, touch.dy * h);
            break;
        }
        case SDL_FINGERMOTION:
        {
            const SDL_TouchFingerEvent &touch = event.tfinger;
            const int w = mainGraphics->mWidth;
            const int h = mainGraphics->mHeight;
            logger->log("event: SDL_FINGERMOTION: %u,%u (%f,%f) (%f,%f)",
                CAST_U32(touch.touchId),
                CAST_U32(touch.fingerId),
                touch.x * w, touch.y * h,
                touch.dx * w, touch.dy * h);
            break;
        }
        case SDL_MULTIGESTURE:
        {
            const SDL_MultiGestureEvent &gesture = event.mgesture;
            const int w = mainGraphics->mWidth;
            const int h = mainGraphics->mHeight;
            logger->log("event: SDL_MULTIGESTURE: %u %f,%f (%f,%f) %d,%d",
                CAST_U32(gesture.touchId),
                gesture.dTheta, gesture.dDist,
                gesture.x * w, gesture.y * h,
                CAST_S32(gesture.numFingers),
                CAST_S32(gesture.padding));
            break;
        }
        case SDL_KEYDOWN:
            logger->log("event: SDL_KEYDOWN: %d,%d", event.key.state,
                event.key.keysym.scancode);
            break;
        case SDL_KEYUP:
            logger->log("event: SDL_KEYUP: %d,%d", event.key.state,
                event.key.keysym.scancode);
            break;
        case SDL_WINDOWEVENT:
        {
            const int data1 = event.window.data1;
            const int data2 = event.window.data2;
            std::string str;
            switch (event.window.event)
            {
                winEventLog(SDL_WINDOWEVENT_NONE, "SDL_WINDOWEVENT_NONE");
                winEventLog(SDL_WINDOWEVENT_SHOWN, "SDL_WINDOWEVENT_SHOWN");
                winEventLog(SDL_WINDOWEVENT_HIDDEN, "SDL_WINDOWEVENT_HIDDEN");
                winEventLog(SDL_WINDOWEVENT_EXPOSED,
                    "SDL_WINDOWEVENT_EXPOSED");
                winEventLog(SDL_WINDOWEVENT_MOVED, "SDL_WINDOWEVENT_MOVED");
                winEventLog(SDL_WINDOWEVENT_RESIZED,
                    "SDL_WINDOWEVENT_RESIZED");
                winEventLog(SDL_WINDOWEVENT_SIZE_CHANGED,
                    "SDL_WINDOWEVENT_SIZE_CHANGED");
                winEventLog(SDL_WINDOWEVENT_MINIMIZED,
                    "SDL_WINDOWEVENT_MINIMIZED");
                winEventLog(SDL_WINDOWEVENT_MAXIMIZED,
                    "SDL_WINDOWEVENT_MAXIMIZED");
                winEventLog(SDL_WINDOWEVENT_RESTORED,
                    "SDL_WINDOWEVENT_RESTORED");
                winEventLog(SDL_WINDOWEVENT_ENTER, "SDL_WINDOWEVENT_ENTER");
                winEventLog(SDL_WINDOWEVENT_LEAVE, "SDL_WINDOWEVENT_LEAVE");
                winEventLog(SDL_WINDOWEVENT_FOCUS_GAINED,
                    "SDL_WINDOWEVENT_FOCUS_GAINED");
                winEventLog(SDL_WINDOWEVENT_FOCUS_LOST,
                    "SDL_WINDOWEVENT_FOCUS_LOST");
                winEventLog(SDL_WINDOWEVENT_CLOSE, "SDL_WINDOWEVENT_CLOSE");
                default:
                    str = "unknown";
                    break;
            }
            logger->log("event: SDL_WINDOWEVENT: %s: %d,%d",
                str.c_str(), data1, data2);
            break;
        }
        case SDL_TEXTINPUT:
        {
            const char *const text = event.text.text;
            logger->log("event: SDL_TEXTINPUT: %s", text);
            const size_t sz = strlen(event.text.text);
            for (size_t f = 0; f < sz; f ++)
                logger->log("dec: %d", text[f]);
            break;
        }
        case SDL_APP_TERMINATING:
            logger->log("SDL_APP_TERMINATING");
            break;
        case SDL_APP_LOWMEMORY:
            logger->log("SDL_APP_LOWMEMORY");
            break;
        case SDL_APP_WILLENTERBACKGROUND:
            logger->log("SDL_APP_WILLENTERBACKGROUND");
            break;
        case SDL_APP_WILLENTERFOREGROUND:
            logger->log("SDL_APP_WILLENTERFOREGROUND");
            break;
        case SDL_APP_DIDENTERFOREGROUND:
            logger->log("SDL_APP_DIDENTERFOREGROUND");
            break;
        case SDL_APP_DIDENTERBACKGROUND:
            logger->log("SDL_APP_DIDENTERBACKGROUND");
            break;
#else
        case SDL_MOUSEMOTION:
            logger->log("event: SDL_MOUSEMOTION: %u,%d,%d",
                event.motion.state, event.motion.x, event.motion.y);
            break;
        case SDL_KEYDOWN:
            logger->log("event: SDL_KEYDOWN: %d,%d,%d", event.key.state,
                event.key.keysym.scancode, event.key.keysym.unicode);
            break;
        case SDL_KEYUP:
            logger->log("event: SDL_KEYUP: %d,%d,%d", event.key.state,
                event.key.keysym.scancode, event.key.keysym.unicode);
            break;
        case SDL_VIDEORESIZE:
            logger->log("event: SDL_VIDEORESIZE");
            break;
        case SDL_VIDEOEXPOSE:
            logger->log("event: SDL_VIDEOEXPOSE");
            break;
        case SDL_ACTIVEEVENT:
            logger->log("event: SDL_ACTIVEEVENT: %d %d",
                event.active.state, event.active.gain);
            break;
#endif
        case SDL_MOUSEBUTTONDOWN:
            logger->log("event: SDL_MOUSEBUTTONDOWN: %d,%d,%d,%d",
                event.button.button, event.button.state,
            event.button.x, event.button.y);
            break;
        case SDL_MOUSEBUTTONUP:
            logger->log("event: SDL_MOUSEBUTTONUP: %d,%d,%d,%d",
                event.button.button, event.button.state,
            event.button.x, event.button.y);
            break;
        case SDL_JOYAXISMOTION:
            logger->log("event: SDL_JOYAXISMOTION: %d,%d,%d",
                event.jaxis.which, event.jaxis.axis, event.jaxis.value);
            break;
        case SDL_JOYBALLMOTION:
            logger->log("event: SDL_JOYBALLMOTION: %d,%d,%d,%d",
                event.jball.which, event.jball.ball,
                event.jball.xrel, event.jball.yrel);
            break;
        case SDL_JOYHATMOTION:
            logger->log("event: SDL_JOYHATMOTION: %d,%d,%d", event.jhat.which,
                event.jhat.hat, event.jhat.value);
            break;
        case SDL_JOYBUTTONDOWN:
            logger->log("event: SDL_JOYBUTTONDOWN: %d,%d,%d",
                event.jbutton.which, event.jbutton.button,
                event.jbutton.state);
            break;
        case SDL_JOYBUTTONUP:
            logger->log("event: SDL_JOYBUTTONUP: %d,%d,%d",
                event.jbutton.which, event.jbutton.button,
                event.jbutton.state);
            break;
        case SDL_QUIT:
            logger->log("event: SDL_QUIT");
            break;
        case SDL_SYSWMEVENT:
            logger->log("event: SDL_SYSWMEVENT");
            break;
        case SDL_USEREVENT:
            logger->log("event: SDL_USEREVENT");
            break;
#ifdef ANDROID
#ifndef USE_SDL2
        case SDL_ACCELEROMETER:
            logger->log("event: SDL_ACCELEROMETER");
            break;
#endif
#endif
        default:
            logger->log("event: other: %u", event.type);
            break;
    };
}

#ifdef USE_SDL2
void EventsManager::handleSDL2WindowEvent(const SDL_Event &event)
{
#ifndef DYECMD
    int fpsLimit = 0;
#endif
    const int eventType = event.window.event;
    const bool inGame = (client->getState() == State::GAME);
    switch (eventType)
    {
        case SDL_WINDOWEVENT_RESIZED:
            WindowManager::resizeVideo(event.window.data1,
                event.window.data2,
                false);
            break;
        case SDL_WINDOWEVENT_ENTER:
            settings.mouseFocused = true;
            break;
        case SDL_WINDOWEVENT_LEAVE:
            settings.mouseFocused = false;
            break;
        case SDL_WINDOWEVENT_FOCUS_GAINED:
            settings.inputFocused = true;
            break;
        case SDL_WINDOWEVENT_FOCUS_LOST:
            settings.inputFocused = false;
            break;
        case SDL_WINDOWEVENT_MINIMIZED:
            WindowManager::setIsMinimized(true);
#ifndef DYECMD
            if (inGame)
            {
                if (localPlayer && !settings.awayMode)
                {
                    fpsLimit = config.getIntValue("altfpslimit");
                    localPlayer->setHalfAway(true);
                }
            }
#endif
            setPriority(false);
            break;
        case SDL_WINDOWEVENT_RESTORED:
        case SDL_WINDOWEVENT_MAXIMIZED:
            WindowManager::setIsMinimized(false);
#ifndef DYECMD
            if (inGame)
            {
                if (localPlayer)
                {
                    if (!settings.awayMode)
                        fpsLimit = config.getIntValue("fpslimit");
                    localPlayer->setHalfAway(false);
                }
            }
#endif
            setPriority(true);
            break;
        default:
            break;
    }

    if (!inGame)
        return;

#ifndef DYECMD
    if (eventType == SDL_WINDOWEVENT_MINIMIZED
        || eventType == SDL_WINDOWEVENT_RESTORED
        || eventType == SDL_WINDOWEVENT_MAXIMIZED)
    {
        if (localPlayer)
        {
            localPlayer->updateStatus();
            localPlayer->updateName();
        }
        Game::instance()->updateFrameRate(fpsLimit);
    }
#endif
}
#else
void EventsManager::handleActive(const SDL_Event &event)
{
#ifndef DYECMD
    int fpsLimit = 0;
    const bool inGame = (client->getState() == State::GAME);
#endif
    if (event.active.state & SDL_APPACTIVE)
    {
        if (event.active.gain)
        {   // window restore
            WindowManager::setIsMinimized(false);
#ifndef DYECMD
            if (inGame && localPlayer)
            {
                if (!settings.awayMode)
                    fpsLimit = config.getIntValue("fpslimit");
                localPlayer->setHalfAway(false);
            }
#endif
            setPriority(true);
        }
        else
        {   // window minimization
#ifdef ANDROID
            client->setState(State::EXIT);
#else
            WindowManager::setIsMinimized(true);
#ifndef DYECMD
            if (inGame && localPlayer && !settings.awayMode)
            {
                fpsLimit = config.getIntValue("altfpslimit");
                localPlayer->setHalfAway(true);
            }
#endif
            setPriority(false);
#endif
        }
#ifndef DYECMD
        if (inGame && localPlayer)
            localPlayer->updateStatus();
#endif
    }
#ifndef DYECMD
    if (inGame && localPlayer)
        localPlayer->updateName();
#endif

    if (event.active.state & SDL_APPINPUTFOCUS)
        settings.inputFocused = event.active.gain;
    if (event.active.state & SDL_APPMOUSEFOCUS)
        settings.mouseFocused = event.active.gain;
#ifndef DYECMD
    if (inGame)
        Game::instance()->updateFrameRate(fpsLimit);
#endif
}
#endif
