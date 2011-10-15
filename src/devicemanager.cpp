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

using namespace boost::algorithm;

#include "log.h"

#include "devicemanager.h"

void deviceManagerClass::add( const xPLDevice& device ) const
{
  writeLog( "deviceManagerClass::add("+device.VDI+")", logLevel::debug );
  if( contains( device.VDI ) )
    update( device );
  else
  {
    string newtag = "device." + device.VDI + ".";
    ptime expires = second_clock::local_time() + minutes( 2*device.Interval + 1 );

    xPLCache->updateEntry(newtag + "vdi", device.VDI, false);
    xPLCache->updateEntry(newtag + "configdone", device.ConfigDone?"true":"false", false);
    xPLCache->updateEntry(newtag + "configmissing", device.ConfigMissing?"true":"false", false);
    xPLCache->updateEntry(newtag + "configsource", device.ConfigSource, false);
    xPLCache->updateEntry(newtag + "configtype", device.ConfigType?"true":"false", false);
    xPLCache->updateEntry(newtag + "waitingconfig", device.WaitingConfig?"true":"false", false);
    xPLCache->updateEntry(newtag + "current", device.Current?"true":"false", false);
    xPLCache->updateEntry(newtag + "configlistsent", device.ConfigListSent?"true":"false", false);
    xPLCache->updateEntry(newtag + "suspended", device.Suspended?"true":"false", false);
    xPLCache->updateEntry(newtag + "interval", lexical_cast<string>(device.Interval), false);
    xPLCache->updateEntry(newtag + "expires", timeConverter( expires ), false);
  }
}

bool deviceManagerClass::remove( const string& deviceTag ) const
{
  writeLog( "deviceManagerClass::remove("+deviceTag+")", logLevel::debug );
  if( contains( deviceTag ) )
  {
    vector<string> list = xPLCache->childNodes( "device." + deviceTag );
    for( vector<string>::const_iterator it = list.begin(); it != list.end(); it++ )
      xPLCache->deleteEntry( *it );
    return true;
  } else
    return false;
}

bool deviceManagerClass::removeConfig( const string& deviceTag ) const
{
  writeLog( "deviceManagerClass::remove("+deviceTag+")", logLevel::debug );
  if( containsConfig( deviceTag ) )
  {
    vector<string> list = xPLCache->childNodes( "config." + deviceTag );
    for( vector<string>::const_iterator it = list.begin(); it != list.end(); it++ )
      xPLCache->deleteEntry( *it );
    return true;
  } else
    return false;
}

xPLDevice deviceManagerClass::getDevice( const string& deviceTag ) const
{
  xPLDevice retval;
  if( contains( deviceTag ) )
  {
    retval.ConfigDone     = xPLCache->objectValue( "device." + deviceTag + ".configdone"     ) == "true";
    retval.ConfigMissing  = xPLCache->objectValue( "device." + deviceTag + ".configmissing"  ) == "true";
    retval.ConfigSource   = xPLCache->objectValue( "device." + deviceTag + ".configsource"   );
    retval.ConfigType     = xPLCache->objectValue( "device." + deviceTag + ".configtype"     ) == "true";
    retval.Current        = xPLCache->objectValue( "device." + deviceTag + ".current"        ) == "true";
    retval.Expires        = timeConverter( xPLCache->objectValue( "device." + deviceTag + ".expires" ) );
    retval.Interval       = atoi(xPLCache->objectValue( "device." + deviceTag + ".interval"       ).c_str());
    retval.VDI            = xPLCache->objectValue( "device." + deviceTag + ".vdi"            );
    retval.WaitingConfig  = xPLCache->objectValue( "device." + deviceTag + ".waitingconfig"  ) == "true";
    retval.ConfigListSent = xPLCache->objectValue( "device." + deviceTag + ".configlistsent" ) == "true";
    retval.Suspended      = xPLCache->objectValue( "device." + deviceTag + ".suspended"      ) == "true";
  }
  return retval;
}

vector<string> deviceManagerClass::getAllDeviceNames() const
{
  vector<string> retval;
  vector<string> nodes = xPLCache->childNodes( "device." );
  for( vector<string>::iterator it = nodes.begin(); it != nodes.end(); ++it )
  {
    if( ends_with( *it, ".vdi" ) )
    {
      retval.push_back( it->substr( 7, it->size() - 7 - 4 ) ); // leave "device." and ".vdi" away
  writeLog( "deviceManagerClass::getAllDeviceNames() = "+ it->substr( 7, it->size() - 7 - 4 ), logLevel::debug );
    }
  }
  return retval;
}

void deviceManagerClass::update( const xPLDevice& device ) const
{
  writeLog( "deviceManagerClass::update("+device.VDI+") - known: "+ (contains( device.VDI )?"true":"false"), logLevel::debug );

  if( contains( device.VDI ) )
    remove( device.VDI );
  writeLog( "deviceManagerClass::update("+device.VDI+") -> add", logLevel::debug );
  add( device );
}

void deviceManagerClass::processConfigList( const xPL_MessagePtr message ) const
{
  string source = string("")
    + xPL_getSourceVendor    ( message ) + "-"
    + xPL_getSourceDeviceID  ( message ) + "."
    + xPL_getSourceInstanceID( message );
  xPLDevice device = getDevice( source );
  writeLog( "deviceManagerClass::processConfigList("+source+") - found ["+device.VDI+"]", logLevel::debug );
  if( "" == device.VDI )
  { // A config list turned up that we haven't asked for...
    // create a new device...
    const char* intervalString = xPL_getMessageNamedValue( message, "interval" );
    int interval = intervalString ? atoi(intervalString) : 5; // default to 5 minutes
    ptime expires = second_clock::local_time() + minutes( 2*interval + 1 );
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
    update( device );
  }

  xPL_NameValueListPtr nvList = xPL_getMessageBody( message );
  xPL_NameValuePairPtr nvPair = NULL;
  int nvCount = xPL_getNamedValueCount(nvList);
  for( int nvIndex = 0; nvIndex < nvCount; nvIndex++ )
  {
    string newtag = "config." + source + ".options.";
    nvPair = xPL_getNamedValuePairAt( nvList, nvIndex );
    string value = nvPair->itemValue; to_lower( value );
    string key   = nvPair->itemName ; to_lower( key   );
    boost::regex re( "([a-z0-9]{1,16})\\[(\\d{1,3})\\]" );
    boost::smatch matches;
    if( boost::regex_match( value, matches, re) )
    {
      xPLCache->updateEntry( newtag + matches[1]           , ""        , false );
      xPLCache->updateEntry( newtag + matches[1] + ".type" , key       , false );
      xPLCache->updateEntry( newtag + matches[1] + ".count", matches[2], false );
    } else {
      xPLCache->updateEntry( newtag + value                , ""        , false );
      xPLCache->updateEntry( newtag + value      + ".type" , key       , false );
      xPLCache->updateEntry( newtag + value      + ".count", ""        , false );
    }
  }

  // send the config response, but only if we have a config (e.g. from the use via XHCP)
  if( xPLCache->exists( "config." + source + ".current" ) )
  {
    xPLCache->updateEntry( "device." + source + ".configdone", "true", false );
    sendConfigResponse( source, false );
  } else {
    // try to get at least the current config
    xPLMessage::namedValueList command_request; command_request.push_back( make_pair( "command", "request" ) );
    xPL->sendMessage( xPL_MESSAGE_COMMAND, 
      xPL_getSourceVendor( message ), xPL_getSourceDeviceID( message ), xPL_getSourceInstanceID( message ),
      "config", "current", command_request );
  }
}

void deviceManagerClass::processConfigHeartBeat( const xPL_MessagePtr message ) const
{
  string source = string("")
    + xPL_getSourceVendor    ( message ) + "-"
    + xPL_getSourceDeviceID  ( message ) + "."
    + xPL_getSourceInstanceID( message );
  xPLDevice device = getDevice( source );
  writeLog( "deviceManagerClass::processConfigHeartBeat("+source+") - found ["+device.VDI+"]", logLevel::debug );
  if( "" == device.VDI )
  { // this handles a new application that identifies itself with a hbeat straight away.
    // it must either be storing it's config locally, can't be configured, or is configured somewhere else.
    const char* intervalString = xPL_getMessageNamedValue( message, "interval" );
    int interval = intervalString ? atoi(intervalString) : 5; // default to 5 minutes
    ptime expires = second_clock::local_time() + minutes( 2*interval + 1 );
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
    update( device );
  } else {
    xPLCache->updateEntry( "device." + source + ".expires", timeConverter( second_clock::local_time() + minutes( 2*device.Interval + 1 ) ), false );
    // FIXME - that line look like that in the VB code - but it seems really strange!
    xPLCache->updateEntry( "device." + source + ".expires.configdone", "objectcache", false );
  }

  // ask device for configuration (if haven't asked it before...)
  if( !device.ConfigListSent )
  {
    xPLMessage::namedValueList command_request; command_request.push_back( make_pair( "command", "request" ) );
    xPL->sendMessage( xPL_MESSAGE_COMMAND, 
      xPL_getSourceVendor( message ), xPL_getSourceDeviceID( message ), xPL_getSourceInstanceID( message ),
      "config", "list", command_request );
    xPLCache->updateEntry( "device." + source + ".configlistsend", "true", false );
  }
}

void deviceManagerClass::processCurrentConfig( const xPL_MessagePtr message ) const
{
  string source = string("")
    + xPL_getSourceVendor    ( message ) + "-"
    + xPL_getSourceDeviceID  ( message ) + "."
    + xPL_getSourceInstanceID( message );
  xPLDevice device = getDevice( source );
  writeLog( "deviceManagerClass::processCurrentConfig("+source+") - found ["+device.VDI+"]", logLevel::debug );
  if( "" == device.VDI )
  { // A current config turned up for a device that we don't know...
    // Processing the current config doesn't make sense as long as we don't know the config list
    // => just treat it like an heartbeat
    return processConfigHeartBeat( message );
  } else {
    xPLCache->updateEntry( "config." + source + ".expires", timeConverter( second_clock::local_time() + minutes( 2*device.Interval + 1 ) ), false );
    xPLCache->updateEntry( "config." + source + ".current", "true", false );
  }

  xPL_NameValueListPtr nvList = xPL_getMessageBody( message );
  xPL_NameValuePairPtr nvPair = NULL;
  int nvCount = xPL_getNamedValueCount(nvList);
  string multiKey;
  int    multiCount = 0;
  for( int nvIndex = 0; nvIndex < nvCount; nvIndex++ )
  {
    nvPair = xPL_getNamedValuePairAt( nvList, nvIndex );
    string value = nvPair->itemValue; to_lower( value );
    string key   = nvPair->itemName ; to_lower( key   );
    string count = xPLCache->objectValue( "config." + source + ".options." + key + ".count" );
    if( "" != count )
    {
      if( multiKey == key )
        multiCount++;
      else
      {
        multiKey = key;
        multiCount = 0;
      }
      char buf[6];
      snprintf( buf, 6, "{%.3d}", multiCount );
      key += buf;
    }
    xPLCache->updateEntry( "config." + source + ".current." + key, value, false );
  }
}

void deviceManagerClass::processHeartbeat( xPL_MessagePtr message ) const
{
  string source = string("")
    + xPL_getSourceVendor    ( message ) + "-"
    + xPL_getSourceDeviceID  ( message ) + "."
    + xPL_getSourceInstanceID( message );

  xPLDevice device = getDevice( source );
  int interval = atoi(string( xPL_getMessageNamedValue( message, "interval" ) ).c_str());
  ptime expires = second_clock::local_time() + minutes( 2*interval + 1 );
  writeLog( "deviceManagerClass::processHeartbeat("+source+") - found ["+device.VDI+"]", logLevel::debug );

  if( "" == device.VDI )
  {
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
    update( device );

    // Throw it a config request anyway, see what turns up..
    xPLMessage::namedValueList command_request; command_request.push_back( make_pair( "command", "request" ) );
    xPL->sendMessage( xPL_MESSAGE_COMMAND, 
      xPL_getSourceVendor( message ), xPL_getSourceDeviceID( message ), xPL_getSourceInstanceID( message ),
      "config", "list"   , command_request );
    /* // that code below was from the VB - but we don't need it as processConfigList will
       // itself send the config.current request...
    xPL->sendMessage( xPL_MESSAGE_COMMAND, 
      xPL_getSourceVendor( message ), xPL_getSourceDeviceID( message ), xPL_getSourceInstanceID( message ),
      "config", "current", command_request );
    */
  } else {
    xPLCache->updateEntry( "device." + source + ".expires", timeConverter( expires ), false );
  }
}

void deviceManagerClass::processRemove( xPL_MessagePtr message ) const
{
  string source = string("")
    + xPL_getSourceVendor    ( message ) + "-"
    + xPL_getSourceDeviceID  ( message ) + "."
    + xPL_getSourceInstanceID( message );

  remove( source );
  removeConfig( source );
}

void deviceManagerClass::sendConfigResponse( const string& source, const bool removeOldValue ) const
{
  writeLog( "deviceManagerClass::sendConfigResponse("+source+", "+(removeOldValue?"true":"false")+")", logLevel::debug );
  boost::regex re( "config\\." + source + "\\.current\\.([a-z0-9]{1,16})(?:\\{([0-9]{1,3})\\})?" );
  vector<string> entries = xPLCache->filterByRegEx( re );
  xPLMessage::namedValueList list;
  for( vector<string>::const_iterator it = entries.begin(); it != entries.end(); ++it )
  {
    boost::smatch matches;
    boost::regex_match( *it, matches, re );
    writeLog( "deviceManagerClass::sendConfigResponse: ["+*it+"] -> ["+matches[1]+"]("+matches[2]+")",logLevel::debug );
    if( matches.size() > 1 )
      list.push_back( make_pair( matches[1], xPLCache->objectValue( *it ) ) );
  }

  if( list.size() > 0 )
  {
    xPL->sendMessage( xPL_MESSAGE_COMMAND, source, "config", "response", list );
    xPLCache->updateEntry( "config." + source + ".configmissing", "false", false );
    xPLCache->updateEntry( "config." + source + ".waitingconfig", "false", false );
    xPLCache->updateEntry( "config." + source + ".configdone"   , "true" , false ); // FIXME the VB code says "false"...
  }

  if( removeOldValue )
  {
    remove( source );
    removeConfig( source );
  }
}

bool deviceManagerClass::storeNewConfig( const string& source, const string& config ) const
{
  writeLog( "deviceManagerClass::storeNewConfig("+source+", "+config+")", logLevel::debug );
  if( !contains( source ) )
    return false;

  vector<string> list;
  split( list, config, is_any_of("\r\n"), token_compress_on );
  string multiKey;
  int    multiCount = 0;
  for( vector<string>::const_iterator it = list.begin(); it != list.end(); ++it )
  {
    vector<string> tags;
    split( tags, *it, is_any_of("=") );
    to_lower( tags[0] );
    to_lower( tags[1] );
    string count = xPLCache->objectValue( "config." + source + ".options." + tags[0] + ".count" );
    if( "" != count )
    {
      if( multiKey != tags[0] )
      {
        multiKey = tags[0];
        multiCount = 0;
      } else
        multiCount++;

      char buf[6];
      snprintf( buf, 6, "{%.3d}", multiCount );
      tags[0] += buf;
    }
    xPLCache->updateEntry( "config." + source + ".current." + tags[0], tags[1], false );
  }
  if( xPLCache->childNodes( "config." + source + ".current" ).size() > 0 )
  {
    sendConfigResponse( source, false ); // FIXME the VB said true - but why should I delete the values that I've just stored?!?
  }
  return true;
}
