/*
    xPLHAL implementation in C++
    Copyright (C) 2009 by Christian Mayer - xpl at ChristianMayer dot de

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef GLOBALS_H
#define GLOBALS_H

#include "boost/filesystem.hpp"

/**
 * Define all global variables:
 */

/** The global cache of xPL messages */
//extern xPLCacheClass xPLCache;
class xPLCacheClass;
extern xPLCacheClass *xPLCache;

class deviceManagerClass;
extern deviceManagerClass *deviceManager;

class xPLHandler;
extern xPLHandler *xPL;

class xPLMessageQueueClass;
extern xPLMessageQueueClass *xPLMessageQueue;

/** The working directories */
extern boost::filesystem::path xPLHalRootFolder;
extern boost::filesystem::path DataFileFolder;
extern boost::filesystem::path ScriptEngineFolder;
extern boost::filesystem::path rulesFolder;

#endif // GLOBALS_H
