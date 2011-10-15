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

#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace std;
using namespace boost::posix_time;

// this is also including the xPL.h with its data structures:
#include "xplhandler.h"

#include "xplcache.h"
#include "globals.h"

struct xPLDevice
{
  string    VDI;            /** \brief vendor / device / instance = unique id                                      */
  ptime     Expires;        /** \brief time expires                                                                */
  int       Interval;       /** \brief current heartbeat interval                                                  */
  bool      ConfigType;     /** \brief true = config. false = hbeat.                                               */
  bool      ConfigDone;     /** \brief false = new waiting check, true = sent/not required                         */
  bool      WaitingConfig;  /** \brief false = waiting check or not needed, true = manual intervention             */
  bool      ConfigListSent; /** \brief Have we asked this device for it's config?                                  */
  string    ConfigSource;   /** \brief v-d.xml / v-d.cache.xml or empty                                            */
  bool      ConfigMissing;  /** \brief true = no config file, no response from device, false = have/waiting config */
  bool      Suspended;      /** \brief lost heartbeat                                                              */
  bool      Current;        /** \brief asked for current                                                           */
};

class deviceManagerClass
{
  public:
    /** \brief Looks if the device deviceTag is known. */
    bool contains( const string& deviceTag ) const
    { return xPLCache->childNodes( "device." + deviceTag ).size() > 0; }

    /** \brief Looks if the config configTag is known. */
    bool containsConfig( const string& configTag ) const
    { return xPLCache->childNodes( "config." + configTag ).size() > 0; }

    void add( const xPLDevice& device ) const;
    bool remove( const string& deviceTag ) const;
    bool removeConfig( const string& deviceTag ) const;

    /** \brief Returns the device deviceTag when it's known or an empty device */
    xPLDevice getDevice( const string& deviceTag ) const;

    /** \brief Returns all known device names */
    vector<string> getAllDeviceNames() const;

    void update( const xPLDevice& device ) const;

    /** \brief Handle the returned config list that someone (porbably we) have asked for */
    void processConfigList     ( const xPL_MessagePtr message ) const;

    /** \brief Called when a new device poped up and is waiting to be configured */
    void processConfigHeartBeat( const xPL_MessagePtr message ) const;

    /** \brief Called when a device sends its configuration */
    void processCurrentConfig  ( const xPL_MessagePtr message ) const;
    void processHeartbeat      ( const xPL_MessagePtr message ) const;
    void processRemove         ( const xPL_MessagePtr message ) const;

    void sendConfigResponse( const string& source, const bool removeOldValue ) const;
    
    /** \brief A new configuration arrived via XHCP, handle it... */
    bool storeNewConfig( const string& source, const string& config ) const;
};

#endif // DEVICEMANAGER_H
