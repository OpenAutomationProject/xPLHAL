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
#include <map>

// this is also including the xPL.h with its data structures:
#include "xplhandler.h"

class IxPLHandler;
class IxPLCacheClass;

#include "xplcache.h"
#include "globals.h"
#include "xpldevice.h"
#include "i_devicemanager.h"

class deviceManagerClass: public IdeviceManagerClass
{
    public:
        deviceManagerClass(IxPLHandler* xplhandler, IxPLCacheClass* xplcache);

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

        /** \brief Handle the returned config list that someone (porbably we) have asked for */
        void processConfigList     ( const xPL_MessagePtr message );

        /** \brief Called when a new device poped up and is waiting to be configured */
        void processConfigHeartBeat( const xPL_MessagePtr message );

        /** \brief Called when a device sends its configuration */
        void processCurrentConfig  ( const xPL_MessagePtr message );
        void processHeartbeat      ( const xPL_MessagePtr message );
        void processRemove         ( const xPL_MessagePtr message );

        void sendConfigResponse( const std::string& source, const bool removeOldValue );

        /** \brief A new configuration arrived via XHCP, handle it... */
        bool storeNewConfig( const std::string& source, const std::string& config );

    private:
        /*! \TODO: move to correct class, maybe xplMessage */
        std::string extractSourceFromXplMessage( xPL_MessagePtr message );
        boost::posix_time::ptime calculateExpireTime(int interval);
        boost::posix_time::ptime calculateExpireTime(const char* string_interval, int *pInterval = 0);

        IxPLHandler* m_xPL;
        IxPLCacheClass*   m_xPLCache;
        std::map<std::string, xPLDevice> mDeviceMap;
};

#endif // DEVICEMANAGER_H
