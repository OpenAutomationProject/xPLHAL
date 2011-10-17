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

#include "globals.h"
#include "xplcache.h"
#include "devicemanager.h"
#include "log.h"
#include "response_codes.h"

#include "xhcpthread.h"

using namespace boost::algorithm;
using boost::asio::ip::tcp;

XHCPThread::XHCPThread( socket_ptr socket )
     : m_stoprequested(false), tab( "\t" ), newLine( "\r\n" ), endMultiLine( ".\r\n" ),
     sock( socket ), quit( false ),
     inMultilineRequest( false ), multilineRequestHandler( 0 ),
     m_thread(boost::bind(&XHCPThread::run, this))
{
}

XHCPThread::~XHCPThread()
{
    writeLog( "XHCPThread::~XHCPThread()", logLevel::debug );
    quit = true;
    m_stoprequested = true;
    m_thread.join();
}

/**
 * Little helper that returns true if str doesn't end with end
 * FIXME: use Boost functionality instead...
 */
bool endDifferent( std::string str, std::string end )
{
    size_t str_l = str.size();
    size_t end_l = end.size();
    if( str_l < end_l ) return true;
    return str.compare( str_l-end_l, end_l, end );
}

void XHCPThread::run()
{
    writeLog( std::string("XHCPThread::run() [")+ lexical_cast<std::string>(sock.use_count()) +"]", logLevel::debug );

    // initialize here to make sure it's done in advance
    // commands[ "ADDEVENT"          ] = &XHCPThread::;
    // commands[ "ADDSINGLEEVENT"    ] = &XHCPThread::;
    commands[ "CAPABILITIES"      ] = &XHCPThread::commandCapabilities;
    // commands[ "CLEARERRLOG"       ] = &XHCPThread::;
    commands[ "DELDEVCONFIG"      ] = &XHCPThread::commandDelDevConfig;
    // commands[ "DELEVENT"          ] = &XHCPThread::;
    commands[ "DELGLOBAL"         ] = &XHCPThread::commandDelGlobal;
    // commands[ "DELRULE"           ] = &XHCPThread::;
    // commands[ "DELSCRIPT"         ] = &XHCPThread::;
    // commands[ "GETCONFIGXML"      ] = &XHCPThread::;
    commands[ "GETDEVCONFIG"      ] = &XHCPThread::commandGetDevConfig;
    commands[ "GETDEVCONFIGVALUE" ] = &XHCPThread::commandGetDevConfigValue;
    // commands[ "GETERRLOG"         ] = &XHCPThread::;
    // commands[ "GETEVENT"          ] = &XHCPThread::;
    // commands[ "GETGLOBAL"         ] = &XHCPThread::;
    // commands[ "GETRULE"           ] = &XHCPThread::;
    // commands[ "GETSCRIPT"         ] = &XHCPThread::;
    // commands[ "GETSETTING"        ] = &XHCPThread::;
    commands[ "LISTALLDEVS"       ] = &XHCPThread::commandListAllDevices;
    commands[ "LISTDEVICES"       ] = &XHCPThread::commandListDevices;
    // commands[ "LISTEVENTS"        ] = &XHCPThread::;
    commands[ "LISTGLOBALS"       ] = &XHCPThread::commandListGlobals;
    commands[ "LISTOPTIONS"       ] = &XHCPThread::commandListOptions;
    commands[ "LISTRULEGROUPS"    ] = &XHCPThread::commandListRuleGroups;
    commands[ "LISTRULES"         ] = &XHCPThread::commandListRules;
    // commands[ "LISTSCRIPTS"       ] = &XHCPThread::;
    // commands[ "LISTSETTINGS"      ] = &XHCPThread::;
    // commands[ "LISTSINGLEEVENTS"  ] = &XHCPThread::;
    // commands[ "LISTSUBS"          ] = &XHCPThread::;
    // commands[ "MODE"              ] = &XHCPThread::;
    // commands[ "PUTCONFIGXML"      ] = &XHCPThread::;
    commands[ "PUTDEVCONFIG"      ] = &XHCPThread::commandPutDevConfig;
    // commands[ "PUTSCRIPT"         ] = &XHCPThread::;
    // commands[ "RELOAD"            ] = &XHCPThread::;
    // commands[ "REPLINFO"          ] = &XHCPThread::;
    // commands[ "RUNRULE"           ] = &XHCPThread::;
    // commands[ "RUNSUB"            ] = &XHCPThread::;
    // commands[ "SENDXAPMSG"        ] = &XHCPThread::;
    commands[ "SENDXPLMSG"        ] = &XHCPThread::commandSendXPlMessage;
    commands[ "SETGLOBAL"         ] = &XHCPThread::commandSetGlobal;
    // commands[ "SETRULE"           ] = &XHCPThread::;
    // commands[ "SETSETTING"        ] = &XHCPThread::;
    commands[ "QUIT"              ] = &XHCPThread::commandQuit;

    boost::asio::streambuf sb;
    boost::system::error_code error; 
    std::string greeting( "200 CHRISM-XPLHAL.SERVER1 Version 0.0 alpha XHCP 1.5 ready\r\n" );

    boost::asio::write(*sock, boost::asio::buffer(greeting));

    while (!quit) {
        // const int TimeoutInCommand =   5 * 1000; // during transmission of an command
        // const int TimeoutExCommand = 300 * 1000; // between different commands
        std::string data;

        while ( (!inMultilineRequest && endDifferent(data, "\r\n") )
                ||    ( inMultilineRequest && endDifferent(data, "\r\n.\r\n") ) ) {
            //int Timeout = (""==data) ? TimeoutExCommand : TimeoutInCommand;

            std::size_t n = boost::asio::read_until(*sock, sb, '\n');
            boost::asio::streambuf::const_buffers_type bufs = sb.data();
            std::string newData(
                    boost::asio::buffers_begin(bufs),
                    boost::asio::buffers_begin(bufs) + n);
            sb.consume(n);
            /*
               std::string newData;
               size_t length = sock->read_some(boost::asio::buffer(newData), error);

               if (error == boost::asio::error::eof)
               break; // Connection closed cleanly by peer.
               else if (error)
               throw boost::system::system_error(error); // Some other error.*/
            data += newData;
        }

        if( inMultilineRequest ) {
            data.erase( data.size()-5 ); // chop

            if( !multilineRequestHandler ) {
                writeLog( "Error: multilineRequestHandler not implemented! Request: [" + data + "]", logLevel::error );
                multilineRequestHandler = &XHCPThread::commandNotRecognised;
            }

            writeLog( "Request: [" + data   + "]", logLevel::debug );
            std::string result = (this->*multilineRequestHandler)( data );
            writeLog( "Result:\n[\n" + result + "]", logLevel::debug );
            //socket.write( result );
            boost::asio::write(*sock, boost::asio::buffer(result));
        } 
        else {
            data.erase( data.size()-2 ); // chop

            size_t spacePos = data.find( ' ' );
            //boost::algorithm::to_upper( data.substr( 0, spacePos ) );
            writeLog( "data.substr( 0, spacePos ) [" + data.substr( 0, spacePos ) + "]["+(std::string::npos == spacePos ? "" : data.substr( spacePos+1 ))+"]", logLevel::error );
            writeLog( "npos: "+lexical_cast<std::string>(std::string::npos) + " - spacePos: + "+lexical_cast<std::string>(spacePos),  logLevel::error );
            //XHCPcommand command    = 0;//commands[ QString(data).section( " ", 0, 0 ).toUpper() ];
            XHCPcommand command    = commands[ boost::algorithm::to_upper_copy( data.substr( 0, spacePos ) ) ];
            std::string parameters = std::string::npos == spacePos ? "" : data.substr( spacePos+1 );

            if( !command ) {
                writeLog( "Error: Command not implemented! Request: [" + data + "]", logLevel::error );
                command = &XHCPThread::commandNotRecognised;
            }

            writeLog( "Request: [" + data   + "]", logLevel::debug );
            std::string result = (this->*command)( parameters );
            writeLog( "Result:\n[\n" + result + "]", logLevel::debug );
            boost::asio::write(*sock, boost::asio::buffer(result));
        }
    }

    sock->close();
    writeLog( "XHCPThread::run() - end", logLevel::debug );
}

std::string XHCPThread::commandNotRecognised( const std::string& parameter ) 
{
  writeLog( "XHCPThread::commandNotRecognised( " + parameter + " )", logLevel::debug );
  return responseCode( 500 );
}

struct XPLHalCapabilities
{
    int xpl_configuration_manager;
    int xap_support;
    char primary_scripting_lang;
    int xpl_determinator;
    bool events;
    char server_platform;
    bool state_tracking;
};

class XHCPCapabilities
{
    public:
        XHCPCapabilities(const XPLHalCapabilities& caps) {
            std::ostringstream os;
            os << "236 ";
            os << caps.xpl_configuration_manager;
            os << caps.xap_support;
            os << caps.primary_scripting_lang;
            os << caps.xpl_determinator;
            os << caps.events;
            os << caps.server_platform;
            os << caps.state_tracking;
            _return = os.str();
        }

        operator std::string() {
            return _return;
        }

    private:
        std::string _return;
};

std::string XHCPThread::commandCapabilities( const std::string& parameter ) 
{
    writeLog( "XHCPThread::commandCapabilities( " + parameter + " )", logLevel::debug );
    if( "" == parameter || "SCRIPTING" == parameter )
    {
        /* <236 retcode> <xPL Config><xAP support><Scripting><Determinator><Events><Platform><State tracking>*/
        return "236 1-011L0\r\n";
    } else {
        // FIXME
        return responseCode( 500 ); // Command not recognised
    }
}

std::string XHCPThread::commandDelGlobal( const std::string& parameter ) 
{
  writeLog( "XHCPThread::commandDelGlobal( " + parameter + " )", logLevel::debug );
  xPLCache->deleteEntry( parameter );
  return responseCode( 233 );
}

std::string XHCPThread::commandDelDevConfig( const std::string& parameter ) 
{
  writeLog( "XHCPThread::commandDelDevConfig( " + parameter + " )", logLevel::debug );
  deviceManager->removeConfig( parameter );
  return responseCode( 235 );
}

std::string XHCPThread::commandGetDevConfig( const std::string& parameter ) 
{
  writeLog( "XHCPThread::commandGetDevConfig( " + parameter + " )", logLevel::debug );
  xPLDevice device = deviceManager->getDevice( parameter );

  if( "" == device.VDI )  // empty string if device doesn't exist
    return responseCode( 417 );

  if( "" == device.ConfigSource )
    return responseCode( 416 );

  std::string retval = responseCode( 217 );
  boost::regex regex( "config\\." + device.VDI + "\\.options\\.([a-z0-9]{1,16})" );
  std::vector<std::string> entries = xPLCache->filterByRegEx( regex );
  for( std::vector<std::string>::const_iterator it = entries.begin(); it != entries.end(); ++it )
  {
    boost::smatch matches;
    if( boost::regex_match( *it, matches, regex ) )
    {
      retval += matches[1]                              + tab;
      retval += xPLCache->objectValue( *it + ".type"  ) + tab;
      retval += xPLCache->objectValue( *it + ".count" ) + newLine;
    }
  }
  retval += endMultiLine;
  return retval;
}

std::string XHCPThread::commandGetDevConfigValue( const std::string& parameter ) 
{
  writeLog( "XHCPThread::commandGetDevConfigValue( " + parameter + " )", logLevel::debug );
  std::vector<std::string> list;
  split( list, parameter, is_any_of( " " ) );
  if( list.size() != 2 ) return responseCode( 501 ); // = syntax error

  std::string retval = responseCode( 234 );
  // do we have to handle mutliple entries?
  std::string count = xPLCache->objectValue( "config." + list[0] + ".options." + list[1] + ".count" );
  if( "" == count )
  { // no
    std::string entry = xPLCache->objectValue( "config." + list[0] + ".current." + list[1] );
    if( "" != entry )
      retval += list[1] + "=" + entry + newLine;
  } else { // yes
    std::vector<std::string> entries = xPLCache->childNodes( "config." + list[0] + ".current." + list[1] );
    for( std::vector<std::string>::const_iterator it = entries.begin(); it != entries.end(); ++it )
    {
      std::string entry = xPLCache->objectValue( *it );
      if( "" != entry )
        retval += list[1] + "=" + entry + newLine;
    }
  }
  retval += endMultiLine;
  return retval;
}

std::string XHCPThread::commandListAllDevices( const std::string& parameter ) 
{
  writeLog( "XHCPThread::commandListAllGlobals( " + parameter + " )", logLevel::debug );
  std::string result = responseCode( 216 );
  std::vector<std::string> names = deviceManager->getAllDeviceNames();
  for( std::vector<std::string>::const_iterator it = names.begin(); it != names.end(); ++it )
    result += *it + newLine;
  result += endMultiLine;
  return result;
}

std::string XHCPThread::commandListDevices( const std::string& parameter ) 
{
  writeLog( "XHCPThread::commandListGlobals( " + parameter + " )", logLevel::debug );
  std::string result = responseCode( 216 );
  std::vector<std::string> names = deviceManager->getAllDeviceNames();
  std::string type = to_lower_copy( parameter );
  for( std::vector<std::string>::const_iterator it = names.begin(); it != names.end(); ++it )
  {
    xPLDevice device = deviceManager->getDevice( *it );
    bool showDevice = true;
    if( "awaitingconfig" == type && !device.ConfigType    ) showDevice = false;
    if( "configured"     == type &&  device.ConfigType    ) showDevice = false;
    if( "missingconfig"  == type && !device.ConfigMissing ) showDevice = false;

    if( showDevice && !device.Suspended )
    {
      result +=                       device.VDI        + tab;
      result += timeConverter(        device.Expires  ) + tab;
      result += lexical_cast<std::string>( device.Interval ) + tab;
      result += (device.ConfigType    ? "Y" : "N")      + tab;
      result += (device.ConfigDone    ? "Y" : "N")      + tab;
      result += (device.WaitingConfig ? "Y" : "N")      + tab;
      result += (device.Suspended     ? "Y" : "N")      + newLine;
    }
  }
  result += endMultiLine;
  return result;
}

std::string XHCPThread::commandListGlobals( const std::string& parameter ) 
{
  writeLog( "XHCPThread::commandListGlobals( " + parameter + " )", logLevel::debug );
  std::string result = responseCode( 231 );
  result += xPLCache->listAllObjects();
  result += endMultiLine;
  return result;
}

std::string XHCPThread::commandListOptions( const std::string& parameter ) 
{
  writeLog( "XHCPThread::commandListOptions( " + parameter + " )", logLevel::debug );
  std::string result;
  if( "" == parameter )
    result = responseCode( 501 );
  else
    // result = responseCode( 205 ); // <-- according to spec
    result = responseCode( 405 );    // <-- accordung to VB code
  
  return result;
}

/**
 * \brief List all determinator groups
 */
std::string XHCPThread::commandListRuleGroups( const std::string& parameter )
{
  writeLog( "XHCPThread::commandListRuleGroups( " + parameter + " )", logLevel::debug );
  std::string result = responseCode( 240 );
  // FIXME add listing of groups
  result += endMultiLine;
  return result;
}

/**
 * \brief List all determinators
 */
std::string XHCPThread::commandListRules( const std::string& parameter )
{
  writeLog( "XHCPThread::commandListRules( " + parameter + " )", logLevel::debug );
  std::string result = responseCode( 237 );
  // FIXME add listing of groups
  result += endMultiLine;
  return result;
}

std::string XHCPThread::commandPutDevConfig( const std::string& parameter )
{
  writeLog( "XHCPThread::commandPutDevConfig( " + parameter + " )", logLevel::debug );
  static std::string device;
  if( inMultilineRequest )
  {
    inMultilineRequest = false;
    multilineRequestHandler = 0;
    if( deviceManager->storeNewConfig( device, parameter ) )
      return responseCode( 220 ); // storing succeded
    else
      return responseCode( 503 ); // storing failed
  }
  if( !deviceManager->contains( parameter ) )
    return responseCode( 417 );

  device = parameter;
  inMultilineRequest = true;
  multilineRequestHandler = &XHCPThread::commandPutDevConfig;
  return responseCode( 320 );
}

std::string XHCPThread::commandSetGlobal( const std::string& parameter )
{
  writeLog( "XHCPThread::commandSetGlobal( " + parameter + " )", logLevel::debug );

  std::vector<std::string> list;
  split( list, parameter, is_any_of( " " ) );
  if( list.size() != 2 ) return responseCode( 501 );

  xPLCache->updateEntry( list[0], list[1] );
  return responseCode( 232 );
}

std::string XHCPThread::commandSendXPlMessage( const std::string& parameter )
{
  writeLog( "XHCPThread::commandSendXPlMessage( " + parameter + " ) [inMultilineRequest="+(inMultilineRequest?"true":"false")+"]", logLevel::debug );
  if( inMultilineRequest )
  {
    inMultilineRequest = false;
    multilineRequestHandler = 0;
    return responseCode( 213 );
  }
  inMultilineRequest = true;
  multilineRequestHandler = &XHCPThread::commandSendXPlMessage;
  // FIXME
  writeLog( "Sending an xPL message via XHCP isn't implemented yet!", logLevel::error );
  return responseCode( 313 );
}

std::string XHCPThread::commandQuit( const std::string& parameter )
{
  writeLog( "XHCPThread::commandQuit( " + parameter + " )", logLevel::debug );
  quit = true;
  return responseCode( 221 );
}
