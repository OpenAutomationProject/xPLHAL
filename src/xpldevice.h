#pragma once
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

#include <string>
#include <chrono>

typedef std::chrono::time_point<std::chrono::steady_clock> steady_time_point;

struct xPLDevice
{
    std::string              VDI;            //! \brief vendor / device / instance = unique id
    steady_time_point        Expires;        //! \brief time expires
    int                      Interval;       //! \brief current heartbeat interval
    bool                     ConfigType;     //! \brief true = config. false = hbeat.
    bool                     ConfigDone;     //! \brief false = new waiting check, true = sent/not required
    bool                     WaitingConfig;  //! \brief false = waiting check or not needed, true = manual intervention
    bool                     ConfigListSent; //! \brief Have we asked this device for it's config?
    std::string              ConfigSource;   //! \brief v-d.xml / v-d.cache.xml or empty
    bool                     ConfigMissing;  //! \brief true = no config file, no response from device, false = have/waiting config */
    bool                     Suspended;      //! \brief lost heartbeat
    bool                     Current;        //! \brief asked for current
};
