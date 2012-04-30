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
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <cstdio>

using boost::algorithm::to_lower;
using boost::algorithm::split;
using boost::algorithm::is_any_of;
using boost::algorithm::token_compress_on;

#include "log.h"
#include "devicemanager.h"
#include "i_xplcache.h"

using std::string;
using std::vector;

DeviceManager::DeviceManager(IxPLCacheClass* xplcache)
:m_xPLCache(xplcache)
{
}


void DeviceManager::add( const xPLDevice& device )
{
    writeLog("DeviceManager::add(" + device.VDI + ")", logLevel::debug);
    if (contains( device.VDI )) {
        mDeviceMap[device.VDI] = device;
    } else {
        mDeviceMap[device.VDI] = device;
        mDeviceMap[device.VDI].Expires = calculateExpireTime(device.Interval);
    }
}

bool DeviceManager::remove( const string& deviceTag )
{
    writeLog( "DeviceManager::remove("+deviceTag+")", logLevel::debug );
    auto device = mDeviceMap.find(deviceTag);
    if (device != mDeviceMap.end()) {
        mDeviceMap.erase(device);
        return true;
    }
    return false;
}

bool DeviceManager::removeConfig( const string& deviceTag ) const
{
    writeLog( "DeviceManager::remove("+deviceTag+")", logLevel::debug );
    if( containsConfig( deviceTag ) ) {
        vector<string> list = m_xPLCache->childNodes( "config." + deviceTag );
        for (auto listitem : list) {
            m_xPLCache->deleteEntry(listitem);
        }
        return true;
    }
    return false;
}

xPLDevice DeviceManager::getDevice( const string& deviceTag ) const
{
    xPLDevice retval;
    auto device = mDeviceMap.find(deviceTag);
    if (device != mDeviceMap.end()) {
        retval = device->second;
    }
    return retval;
}

vector<string> DeviceManager::getAllDeviceNames() const
{
    vector<string> retval;
    retval.reserve(mDeviceMap.size());
    for (auto node : mDeviceMap) {
        retval.push_back(node.first);
        writeLog( "DeviceManager::getAllDeviceNames() = "+ node.first, logLevel::debug );
    }
    return retval;
}
        
void DeviceManager::processXplMessage( const xPLMessagePtr message ) 
{
    string schema = message->msgClass + string(".") + message->msgType;
    if( message->type != xPL_MESSAGE_COMMAND )
    {
        if(        "config.list"    == schema )
        {
            // someone (probably we) have asked for the config list - handle it now...
            processConfigList( message );
        } else if( "config.current" == schema )
        {
            // someone requested the device to send it's current configuration
            processCurrentConfig( message );
        } else if( "config.app"     == schema || "config.basic" == schema )
        {
            // a new device poped up and wants to be configured
            processConfigHeartBeat( message );
        } else if( "hbeat.basic"    == schema || "hbeat.app"    == schema )
        {
            /*
               If msgSource = MySourceTag Then
               RaiseEvent AddtoCache("xplhal." & msgSource & ".alive", Now.ToString, False)
               End If
             */
            processHeartbeat( message );
        } else if( "hbeat.end"      == schema )
        {
            processRemove( message );
        }
    }
}

void DeviceManager::processConfigList( const xPLMessagePtr message ) 
{
    string source = message->getSourceVDI();
    xPLDevice device = getDevice( source );
    writeLog( "DeviceManager::processConfigList("+source+") - found ["+device.VDI+"]", logLevel::debug );
    if( "" == device.VDI ) {
        // A config list turned up that we haven't asked for...
        // create a new device...
        int interval = 5;
        steady_time_point expires = calculateExpireTime(message->getNamedValue("interval"), &interval);
        device.VDI            = source;        // vendor / device / instance = unique id
        device.ConfigDone     = false;         // false = new waiting check, true = sent/not required
        device.ConfigMissing  = false;         // true = no config file, no response from device, false = have/waiting config
        device.ConfigSource   = "objectcache"; // v-d.xml / v-d.cache.xml or empty
        device.ConfigType     = true;          // true = config. false = hbeat.
        device.WaitingConfig  = false;         // false = waiting check or not needed, true = manual intervention
        device.Current        = false;         // asked for current config
        device.ConfigListSent = true;          // Have we asked this device for it's config?
        device.Suspended      = false;         // lost heartbeat
        device.Interval       = interval;      // current heartbeat interval
        device.Expires        = expires;       // time expires
        mDeviceMap[device.VDI] = device;
    }

    for( auto entry : message->namedValues) {
        string newtag = "config." + source + ".options.";
        string value = entry.first;
        to_lower(value);
        string key   = entry.second;
        to_lower(key);
        boost::regex re( "([a-z0-9]{1,16})\\[(\\d{1,3})\\]" );
        boost::smatch matches;
        if (boost::regex_match(value, matches, re)) {
            m_xPLCache->updateEntry( newtag + matches[1]           , ""        , false );
            m_xPLCache->updateEntry( newtag + matches[1] + ".type" , key       , false );
            m_xPLCache->updateEntry( newtag + matches[1] + ".count", matches[2], false );
        } 
        else {
            m_xPLCache->updateEntry( newtag + value                , ""        , false );
            m_xPLCache->updateEntry( newtag + value      + ".type" , key       , false );
            m_xPLCache->updateEntry( newtag + value      + ".count", ""        , false );
        }
    }

    // send the config response, but only if we have a config (e.g. from the use via XHCP)
    if( m_xPLCache->exists( "config." + source + ".current" ) ) {
        mDeviceMap[source].ConfigDone = true;
        sendConfigResponse( source, false );
    } 
    else {
        // try to get at least the current config
        xPLMessagePtr msg( new xPLMessage(xPL_MESSAGE_COMMAND,
                                          source,
                                          "config", "current",
                                          {{"command", "request"}}) );
        m_sigSendXplMessage(msg);
    }
}

steady_time_point DeviceManager::calculateExpireTime(const string& string_interval, int *pInterval)
{
    int interval = string_interval.empty() ? atoi(string_interval.c_str()) : 5; // default to 5 minutes
    if (pInterval) {
        *pInterval = interval;
    }
    return calculateExpireTime(interval);
}

steady_time_point DeviceManager::calculateExpireTime(int interval)
{
    return std::chrono::steady_clock::now() + std::chrono::minutes( 2* interval + 1 );
}

void DeviceManager::processConfigHeartBeat( const xPLMessagePtr message )
{
    string source = message->getSourceVDI();

    xPLDevice device = getDevice( source );
    writeLog( "DeviceManager::processConfigHeartBeat("+source+") - found ["+device.VDI+"]", logLevel::debug );
    if( device.VDI.empty() ) {
        // this handles a new application that identifies itself with a hbeat straight away.
        // it must either be storing it's config locally, can't be configured, or is configured somewhere else.
        int interval = 5;
        steady_time_point expires = calculateExpireTime(message->getNamedValue("interval"), &interval);
        device.VDI            = source;       // vendor / device / instance = unique id
        device.ConfigDone     = false;        // false = new waiting check, true = sent/not required
        device.ConfigMissing  = true;         // true = no config file, no response from device, false = have/waiting config
        device.ConfigSource   = "xplmessage"; // v-d.xml / v-d.cache.xml or empty
        device.ConfigType     = true;         // true = config. false = hbeat.
        device.WaitingConfig  = true;         // false = waiting check or not needed, true = manual intervention
        device.Current        = false;        // asked for current config
        device.ConfigListSent = false;        // Have we asked this device for it's config?
        device.Suspended      = false;        // lost heartbeat
        device.Interval       = interval;     // current heartbeat interval
        device.Expires        = expires;      // time expires
        mDeviceMap[device.VDI] = device;
    } else {
        mDeviceMap[device.VDI].Expires = calculateExpireTime(device.Interval);
        mDeviceMap[device.VDI].ConfigSource = "objectcache";
        // FIXME - that line look like that in the VB code - but it seems really strange!
//        m_xPLCache->updateEntry( "device." + source + ".expires.configdone", "objectcache", false );
    }

    // ask device for configuration (if haven't asked it before...)
    if( !device.ConfigListSent ) {
        xPLMessagePtr msg( new xPLMessage(xPL_MESSAGE_COMMAND,
                                          source,
                                          "config", "list",
                                          {{"command", "request"}}));
        m_sigSendXplMessage(msg);
        mDeviceMap[device.VDI].ConfigListSent = true;
    }
}

void DeviceManager::processCurrentConfig( const xPLMessagePtr message )
{
    string source = message->getSourceVDI();
    xPLDevice device = getDevice( source );
    writeLog( "DeviceManager::processCurrentConfig("+source+") - found ["+device.VDI+"]", logLevel::debug );
    if( "" == device.VDI ) {
        // A current config turned up for a device that we don't know...
        // Processing the current config doesn't make sense as long as we don't know the config list
        // => just treat it like an heartbeat
        return processConfigHeartBeat( message );
    } else {
//        steady_time_point expires = calculateExpireTime(device.Interval);
//        m_xPLCache->updateEntry( "config." + source + ".expires", timeConverter(calculateExpireTime(device.Interval), false ));
        m_xPLCache->updateEntry( "config." + source + ".current", "true", false );
    }
    
    string multiKey;
    int multiCount = 0;
    for( auto entry : message->namedValues) {
        string value = entry.first;
        to_lower(value);
        string key   = entry.second;
        to_lower(key);
        string count = m_xPLCache->objectValue( "config." + source + ".options." + key + ".count" );
        if( "" != count ) {
            if( multiKey == key ) {
                multiCount++;
            } else {
                multiKey = key;
                multiCount = 0;
            }
            char buf[6];
            snprintf( buf, 6, "{%.3d}", multiCount );
            key += buf;
        }
        m_xPLCache->updateEntry( "config." + source + ".current." + key, value, false );
    }
}

void DeviceManager::processHeartbeat( xPLMessagePtr message )
{
    string source = message->getSourceVDI();

    xPLDevice device = getDevice( source );
    int interval = 5;
    steady_time_point expires = calculateExpireTime(message->getNamedValue("interval"), &interval);
    writeLog( "DeviceManager::processHeartbeat("+source+") - found ["+device.VDI+"]", logLevel::debug );

    if( "" == device.VDI ) {
        // this handles a new application that identifies itself with a hbeat straight away.
        // it must either be storing it's config locally, can't be configured, or is configured somewhere else.
        device.VDI            = source;        // vendor / device / instance = unique id
        device.ConfigDone     = true;          // false = new waiting check, true = sent/not required
        device.ConfigMissing  = true;          // true = no config file, no response from device, false = have/waiting config
        device.ConfigSource   = "objectcache"; // v-d.xml / v-d.cache.xml or empty
        device.ConfigType     = false;         // true = config. false = hbeat.
        device.WaitingConfig  = false;         // false = waiting check or not needed, true = manual intervention
        device.Current        = false;         // asked for current config
        device.ConfigListSent = true;          // Have we asked this device for it's config?
        device.Suspended      = false;         // lost heartbeat
        device.Interval       = interval;      // current heartbeat interval
        device.Expires        = expires;       // time expires
        mDeviceMap[device.VDI] = device;

        // Throw it a config request anyway, see what turns up..
        xPLMessagePtr msg( new xPLMessage(xPL_MESSAGE_COMMAND,
                                          source,
                                          "config", "list",
                                          {{"command", "request"}} ));
        m_sigSendXplMessage(msg);
        /* // that code below was from the VB - but we don't need it as processConfigList will
        // itself send the config.current request...
        m_xPL->sendMessage( xPL_MESSAGE_COMMAND, 
        xPL_getSourceVendor( message ), xPL_getSourceDeviceID( message ), xPL_getSourceInstanceID( message ),
        "config", "current", command_request );
         */
    } else {
        mDeviceMap[source].Expires = expires;
    }
}

void DeviceManager::processRemove( xPLMessagePtr message )
{
    string source = message->getSourceVDI();

    remove( source );
    removeConfig( source );
}

void DeviceManager::sendConfigResponse( const string& source, const bool removeOldValue )
{
    writeLog( "DeviceManager::sendConfigResponse("+source+", "+(removeOldValue?"true":"false")+")", logLevel::debug );
    boost::regex re( "config\\." + source + "\\.current\\.([a-z0-9]{1,16})(?:\\{([0-9]{1,3})\\})?" );
    vector<string> entries = m_xPLCache->filterByRegEx( re );
    xPLMessage::namedValueList list;
    for( auto entry : entries) {
        boost::smatch matches;
        boost::regex_match( entry, matches, re );
        writeLog( "DeviceManager::sendConfigResponse: ["+entry+"] -> ["+matches[1]+"]("+matches[2]+")",logLevel::debug );
        if( matches.size() > 1 ) {
            list.push_back( std::make_pair( matches[1], m_xPLCache->objectValue( entry ) ) );
        }
    }

    if( list.size() > 0 ) {
        xPLMessagePtr msg(  new xPLMessage(xPL_MESSAGE_COMMAND,
                            source,
                            "config", "response",
                            list) );
        m_sigSendXplMessage(msg);
        m_xPLCache->updateEntry( "config." + source + ".configmissing", "false", false );
        m_xPLCache->updateEntry( "config." + source + ".waitingconfig", "false", false );
        m_xPLCache->updateEntry( "config." + source + ".configdone"   , "true" , false ); // FIXME the VB code says "false"...
    }

    if( removeOldValue ) {
        remove( source );
        removeConfig( source );
    }
}

bool DeviceManager::storeNewConfig( const string& source, const string& config )
{
    writeLog( "DeviceManager::storeNewConfig("+source+", "+config+")", logLevel::debug );
    auto device = mDeviceMap.find(source);
    if (device == mDeviceMap.end()) {
        return false;
    }

    vector<string> list;
    split( list, config, is_any_of("\r\n"), token_compress_on );
    string multiKey;
    int multiCount = 0;
    for( auto listitem : list) {
        vector<string> tags;
        split( tags, listitem, is_any_of("=") );
        to_lower( tags[0] );
        to_lower( tags[1] );
        string count = m_xPLCache->objectValue( "config." + source + ".options." + tags[0] + ".count" );
        if( "" != count ) {
            if( multiKey != tags[0] ) {
                multiKey = tags[0];
                multiCount = 0;
            } else {
                multiCount++;
            }

            char buf[6];
            snprintf( buf, 6, "{%.3d}", multiCount );
            tags[0] += buf;
        }
        m_xPLCache->updateEntry( "config." + source + ".current." + tags[0], tags[1], false );
    }
    if( m_xPLCache->childNodes( "config." + source + ".current" ).size() > 0 ) {
        sendConfigResponse( source, false ); // FIXME the VB said true - but why should I delete the values that I've just stored?
    }
    return true;
}
    
bool DeviceManager::contains( const string& deviceTag ) const
{
    return mDeviceMap.find(deviceTag) != mDeviceMap.end();
}

bool DeviceManager::containsConfig( const string& configTag ) const
{
    return m_xPLCache->childNodes( "config." + configTag ).size() > 0; 
}

