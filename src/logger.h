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

#ifndef LOGGER_H
#define LOGGER_H

#ifdef USE_SDL2
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-default"
#endif
#include <SDL_stdinc.h>
_SDL_stdinc_h
#ifdef USE_SDL2
#pragma GCC diagnostic pop
#endif
#include <SDL_mutex.h>

#include <fstream>
#include <vector>

#include "localconsts.h"

#ifdef ENABLEDEBUGLOG
#define DEBUGLOG(str) \
    if (logger && !mIgnore) \
        logger->dlog(str)
#define DEBUGLOG2(str, pos, comment) \
    if (logger && !mIgnore) \
        logger->dlog2(str, pos, comment)
#define DEBUGLOGSTR(str) \
    if (logger) \
        logger->dlog(str)
#define IGNOREDEBUGLOG mIgnore = Net::isIgnorePacket(mId)
#else
#define DEBUGLOG(str) {}
#define DEBUGLOG2(str, comment) {}
#define DEBUGLOGSTR(str)
#define IGNOREDEBUGLOG {}
#endif

#define UNIMPLIMENTEDPACKET \
    logger->unimplimented(msg.getId())

#define UNIMPLIMENTEDPACKETFIELD(field) \
    logger->unimplimented(msg.getId(), field)

#define WRONGPACKETSIZE \
    logger->unimplimented(CAST_U32(mId), mLength, mPos)

/**
 * The Log Class : Useful to write debug or info messages
 */
class Logger final
{
    public:
        /**
         * Constructor.
         */
        Logger();

        A_DELETE_COPY(Logger)

        /**
         * Destructor, closes log file.
         */
        ~Logger();

        /**
         * Sets the file to log to and opens it
         */
        void setLogFile(const std::string &logFilename);

        /**
         * Sets whether the log should be written to standard output.
         */
        void setLogToStandardOut(const bool value)
        { mLogToStandardOut = value; }

        /**
         * Enters a message in the log. The message will be timestamped.
         */
        void log(const char *const log_text, ...) A_NONNULL(2)
#ifdef __GNUC__
#ifdef __OpenBSD__

            __attribute__((__format__(printf, 2, 3)))
#else  // __OpenBSD__

#ifdef ENABLE_CILKPLUS
            __attribute__((__format__(gnu_printf, 1, 2)))
#else  // ENABLE_CILKPLUS

            __attribute__((__format__(gnu_printf, 2, 3)))
#endif  // ENABLE_CILKPLUS

#endif  // __OpenBSD__
#endif  // __GNUC__
            ;

        /**
         * Enters a message in the log. The message will be timestamped.
         */
        void assertLog(const char *const log_text, ...) A_NONNULL(2)
#ifdef __GNUC__
#ifdef __OpenBSD__

            __attribute__((__format__(printf, 2, 3)))
#else  // __OpenBSD__

#ifdef ENABLE_CILKPLUS
            __attribute__((__format__(gnu_printf, 1, 2)))
#else  // ENABLE_CILKPLUS

            __attribute__((__format__(gnu_printf, 2, 3)))
#endif  // ENABLE_CILKPLUS

#endif  // __OpenBSD__
#endif  // __GNUC__
            ;

        /**
         * Enters a message in the log (thread safe).
         */
        void log_r(const char *const log_text, ...) A_NONNULL(2)
#ifdef __GNUC__
#ifdef __OpenBSD__
            __attribute__((__format__(printf, 2, 3)))
#else  // __OpenBSD__

#ifdef ENABLE_CILKPLUS
            __attribute__((__format__(gnu_printf, 1, 2)))
#else  // ENABLE_CILKPLUS

            __attribute__((__format__(gnu_printf, 2, 3)))
#endif  // ENABLE_CILKPLUS

#endif  // __OpenBSD__
#endif  // __GNUC__
            ;

        /**
         * Enters a message in the log. The message will be timestamped.
         */
        void log1(const char *const log_text) A_NONNULL(2);

        /**
         * Enters a message in the log. The message will be timestamped.
         */
        void log(const std::string &str);

        void flush();

#ifdef ENABLEDEBUGLOG
        /**
         * Enters debug message in the log. The message will be timestamped.
         */
        void dlog(const std::string &str);

        void dlog2(const std::string &str,
                   const int pos,
                   const char* const comment);
#endif

        void setDebugLog(const bool n)
        { mDebugLog = n; }

        void setReportUnimplimented(const bool n)
        { mReportUnimplimented = n; }

        /**
         * Log an error and quit. The error will pop-up on Windows and Mac, and
         * will be printed to standard error everywhere else.
         */
        void error(const std::string &error_text)
#ifndef ENABLE_CILKPLUS
        __attribute__ ((noreturn))
#endif
        ;

        /**
         * Log an error and quit. The error will pop-up on Windows and Mac, and
         * will be printed to standard error everywhere else.
         */
        void safeError(const std::string &error_text)
#ifndef ENABLE_CILKPLUS
        __attribute__ ((noreturn))
#endif
        ;

        void unimplimented(const int id);

        void unimplimented(const int id,
                           const int id2);

        void unimplimented(const uint32_t id,
                           const uint32_t id2,
                           const uint32_t id3) const;

    private:
        std::ofstream mLogFile;
        std::vector<std::string> mDelayedLog;
        SDL_mutex *mMutex;
        volatile bool mThreadLocked;
        bool mLogToStandardOut;
        bool mDebugLog;
        bool mReportUnimplimented;
};

extern Logger *logger;

#endif  // LOGGER_H
