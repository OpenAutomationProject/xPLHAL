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
#include <map>

class IxPLCacheClass;

#include "xplhandler.h"
#include "xplcache.h"
#include "globals.h"
#include "xpldevice.h"

class DeviceManager
{
    public:
        DeviceManager(IxPLCacheClass* xplcache);
        
        /** \brief Looks if the device deviceTag is known. */
        bool contains( const std::string& deviceTag ) const;

        /** \brief Looks if the config configTag is known. */
        bool containsConfig( const std::string& configTag ) const;

        void add( const xPLDevice& device );
        bool remove( const std::string& deviceTag );
        bool removeConfig( const std::string& deviceTag ) const;

        /** \brief Returns the device deviceTag when it's known or an empty device */
        xPLDevice getDevice( const std::string& deviceTag ) const;

        /** \brief Returns all known device names */
        std::vector<std::string> getAllDeviceNames() const;

        /** \brief process incoming xpl messages and check if they are for us */
        void processXplMessage( const xPLMessagePtr message );

        /** \brief Handle the returned config list that someone (porbably we) have asked for */
        void processConfigList     ( const xPLMessagePtr message );

        /** \brief Called when a new device poped up and is waiting to be configured */
        void processConfigHeartBeat( const xPLMessagePtr message );

        /** \brief Called when a device sends its configuration */
        void processCurrentConfig  ( const xPLMessagePtr message );
        void processHeartbeat      ( const xPLMessagePtr message );
        void processRemove         ( const xPLMessagePtr message );

        void sendConfigResponse( const std::string& source, const bool removeOldValue );

        /** \brief A new configuration arrived via XHCP, handle it... */
        bool storeNewConfig( const std::string& source, const std::string& config );
        
        xPLHandler::signal_t m_sigSendXplMessage;
    private:
        boost::posix_time::ptime calculateExpireTime(int interval);
        boost::posix_time::ptime calculateExpireTime(const std::string& string_interval, int *pInterval = 0);

        IxPLCacheClass*   m_xPLCache;
        std::map<std::string, xPLDevice> mDeviceMap;
};
