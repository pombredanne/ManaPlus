/*
 *  The ManaPlus Client
 *  Copyright (C) 2013-2016  The ManaPlus Developers
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

#ifndef UTILS_FILES_H
#define UTILS_FILES_H

#include "utils/stringvector.h"

#include "localconsts.h"

namespace Files
{
#ifdef ANDROID
    void extractLocale();
#endif

#if defined(ANDROID) || defined(__native_client__)
    typedef void (*CopyFileCallbackPtr) (int cnt);

    void setCopyCallBack(CopyFileCallbackPtr callback);

    void copyPhysFsFile(const std::string &restrict inFile,
                        const std::string &restrict outFile);

    void copyPhysFsDir(const std::string &restrict inDir,
                       const std::string &restrict outDir);

    void extractZip(const std::string &restrict zipName,
                    const std::string &restrict inDir,
                    const std::string &restrict outDir);
#endif  // ANDROID __native_client__

    int renameFile(const std::string &restrict pFrom,
                   const std::string &restrict pTo);

    int copyFile(const std::string &restrict pFrom,
                 const std::string &restrict pTo);

    void getFiles(const std::string &path, StringVect &list);

    void getDirs(const std::string &path, StringVect &list);

    void getFilesWithDir(const std::string &restrict path,
                         StringVect &restrict list);

    bool existsLocal(const std::string &path);

    std::string getPath(const std::string &file);

    bool loadTextFile(const std::string &fileName,
                      StringVect &lines);

    std::string loadTextFileString(const std::string &fileName);

    bool loadTextFileLocal(const std::string &fileName,
                           StringVect &lines);

    void saveTextFile(std::string path,
                      const std::string &restrict name,
                      const std::string &restrict text);

    void deleteFilesInDirectory(std::string path);

    void getFilesInDir(const std::string &dir,
                       StringVect &list,
                       const std::string &ext);

}  // namespace Files

#endif  // UTILS_FILES_H
