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

using boost::algorithm::to_lower;
using boost::algorithm::split;
using boost::algorithm::is_any_of;
using boost::algorithm::token_compress_on;
using boost::algorithm::to_lower_copy;

using boost::asio::ip::tcp;
using std::string;
using std::vector;

XHCPThread::XHCPThread( socket_ptr socket, DeviceManager* dm )
     : m_stoprequested(false)
     , tab( "\t" )
     , newLine( "\r\n" )
     , endMultiLine( ".\r\n" )
     , sock( socket )
     , quit( false )
     , inMultilineRequest( false )
     , multilineRequestHandler( 0 )
     , m_thread(boost::bind(&XHCPThread::run, this))
     , m_deviceManager(dm)
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
bool endDifferent( string str, string end )
{
    size_t str_l = str.size();
    size_t end_l = end.size();
    if( str_l < end_l ) return true;
    return str.compare( str_l-end_l, end_l, end );
}

void XHCPThread::run()
{
    writeLog( string("XHCPThread::run() [")+ lexical_cast<string>(sock.use_count()) +"]", logLevel::debug );

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
    string greeting( "200 CHRISM-XPLHAL.SERVER1 Version 0.0 alpha XHCP 1.5 ready\r\n" );

    boost::asio::write(*sock, boost::asio::buffer(greeting));

    while (!quit) {
        // const int TimeoutInCommand =   5 * 1000; // during transmission of an command
        // const int TimeoutExCommand = 300 * 1000; // between different commands
        string data;

        while ( (!inMultilineRequest && endDifferent(data, "\r\n") )
                ||    ( inMultilineRequest && endDifferent(data, "\r\n.\r\n") ) ) {
            //int Timeout = (""==data) ? TimeoutExCommand : TimeoutInCommand;

            std::size_t n = boost::asio::read_until(*sock, sb, '\n');
            boost::asio::streambuf::const_buffers_type bufs = sb.data();
            string newData(
                    boost::asio::buffers_begin(bufs),
                    boost::asio::buffers_begin(bufs) + n);
            sb.consume(n);
            /*
               string newData;
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
            string result = (this->*multilineRequestHandler)( data );
            writeLog( "Result:\n[\n" + result + "]", logLevel::debug );
            //socket.write( result );
            boost::asio::write(*sock, boost::asio::buffer(result));
        } 
        else {
            data.erase( data.size()-2 ); // chop

            size_t spacePos = data.find( ' ' );
            //boost::algorithm::to_upper( data.substr( 0, spacePos ) );
            writeLog( "data.substr( 0, spacePos ) [" + data.substr( 0, spacePos ) + "]["+(string::npos == spacePos ? "" : data.substr( spacePos+1 ))+"]", logLevel::error );
            writeLog( "npos: "+lexical_cast<string>(string::npos) + " - spacePos: + "+lexical_cast<string>(spacePos),  logLevel::error );
            //XHCPcommand command    = 0;//commands[ QString(data).section( " ", 0, 0 ).toUpper() ];
            XHCPcommand command    = commands[ boost::algorithm::to_upper_copy( data.substr( 0, spacePos ) ) ];
            string parameters = string::npos == spacePos ? "" : data.substr( spacePos+1 );

            if( !command ) {
                writeLog( "Error: Command not implemented! Request: [" + data + "]", logLevel::error );
                command = &XHCPThread::commandNotRecognised;
            }

            writeLog( "Request: [" + data   + "]", logLevel::debug );
            string result = (this->*command)( parameters );
            writeLog( "Result:\n[\n" + result + "]", logLevel::debug );
            boost::asio::write(*sock, boost::asio::buffer(result));
        }
    }

    sock->close();
    writeLog( "XHCPThread::run() - end", logLevel::debug );
}

string XHCPThread::commandNotRecognised( const string& parameter ) 
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

        operator string() {
            return _return;
        }

    private:
        string _return;
};

string XHCPThread::commandCapabilities( const string& parameter ) 
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

string XHCPThread::commandDelGlobal( const string& parameter ) 
{
  writeLog( "XHCPThread::commandDelGlobal( " + parameter + " )", logLevel::debug );
  xPLCache->deleteEntry( parameter );
  return responseCode( 233 );
}

string XHCPThread::commandDelDevConfig( const string& parameter ) 
{
  writeLog( "XHCPThread::commandDelDevConfig( " + parameter + " )", logLevel::debug );
  m_deviceManager->removeConfig( parameter );
  return responseCode( 235 );
}

string XHCPThread::commandGetDevConfig( const string& parameter ) 
{
  writeLog( "XHCPThread::commandGetDevConfig( " + parameter + " )", logLevel::debug );
  xPLDevice device = m_deviceManager->getDevice( parameter );

  if( "" == device.VDI )  // empty string if device doesn't exist
    return responseCode( 417 );

  if( "" == device.ConfigSource )
    return responseCode( 416 );

  string retval = responseCode( 217 );
  boost::regex regex( "config\\." + device.VDI + "\\.options\\.([a-z0-9]{1,16})" );
  vector<string> entries = xPLCache->filterByRegEx( regex );
  for( vector<string>::const_iterator it = entries.begin(); it != entries.end(); ++it )
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

string XHCPThread::commandGetDevConfigValue( const string& parameter ) 
{
  writeLog( "XHCPThread::commandGetDevConfigValue( " + parameter + " )", logLevel::debug );
  vector<string> list;
  split( list, parameter, is_any_of( " " ) );
  if( list.size() != 2 ) return responseCode( 501 ); // = syntax error

  string retval = responseCode( 234 );
  // do we have to handle mutliple entries?
  string count = xPLCache->objectValue( "config." + list[0] + ".options." + list[1] + ".count" );
  if( "" == count )
  { // no
    string entry = xPLCache->objectValue( "config." + list[0] + ".current." + list[1] );
    if( "" != entry )
      retval += list[1] + "=" + entry + newLine;
  } else { // yes
    vector<string> entries = xPLCache->childNodes( "config." + list[0] + ".current." + list[1] );
    for( vector<string>::const_iterator it = entries.begin(); it != entries.end(); ++it )
    {
      string entry = xPLCache->objectValue( *it );
      if( "" != entry )
        retval += list[1] + "=" + entry + newLine;
    }
  }
  retval += endMultiLine;
  return retval;
}

string XHCPThread::commandListAllDevices( const string& parameter ) 
{
  writeLog( "XHCPThread::commandListAllGlobals( " + parameter + " )", logLevel::debug );
  string result = responseCode( 216 );
  vector<string> names = m_deviceManager->getAllDeviceNames();
  for( vector<string>::const_iterator it = names.begin(); it != names.end(); ++it )
    result += *it + newLine;
  result += endMultiLine;
  return result;
}

string XHCPThread::commandListDevices( const string& parameter ) 
{
  writeLog( "XHCPThread::commandListGlobals( " + parameter + " )", logLevel::debug );
  string result = responseCode( 216 );
  vector<string> names = m_deviceManager->getAllDeviceNames();
  string type = to_lower_copy( parameter );
  for( vector<string>::const_iterator it = names.begin(); it != names.end(); ++it )
  {
    xPLDevice device = m_deviceManager->getDevice( *it );
    bool showDevice = true;
    if( "awaitingconfig" == type && !device.ConfigType    ) showDevice = false;
    if( "configured"     == type &&  device.ConfigType    ) showDevice = false;
    if( "missingconfig"  == type && !device.ConfigMissing ) showDevice = false;

    if( showDevice && !device.Suspended )
    {
      result +=                       device.VDI        + tab;
      result += timeConverter(        device.Expires  ) + tab;
      result += lexical_cast<string>( device.Interval ) + tab;
      result += (device.ConfigType    ? "Y" : "N")      + tab;
      result += (device.ConfigDone    ? "Y" : "N")      + tab;
      result += (device.WaitingConfig ? "Y" : "N")      + tab;
      result += (device.Suspended     ? "Y" : "N")      + newLine;
    }
  }
  result += endMultiLine;
  return result;
}

string XHCPThread::commandListGlobals( const string& parameter ) 
{
  writeLog( "XHCPThread::commandListGlobals( " + parameter + " )", logLevel::debug );
  string result = responseCode( 231 );
  result += xPLCache->listAllObjects();
  result += endMultiLine;
  return result;
}

string XHCPThread::commandListOptions( const string& parameter ) 
{
  writeLog( "XHCPThread::commandListOptions( " + parameter + " )", logLevel::debug );
  string result;
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
string XHCPThread::commandListRuleGroups( const string& parameter )
{
  writeLog( "XHCPThread::commandListRuleGroups( " + parameter + " )", logLevel::debug );
  string result = responseCode( 240 );
  // FIXME add listing of groups
  result += endMultiLine;
  return result;
}

/**
 * \brief List all determinators
 */
string XHCPThread::commandListRules( const string& parameter )
{
  writeLog( "XHCPThread::commandListRules( " + parameter + " )", logLevel::debug );
  string result = responseCode( 237 );
  // FIXME add listing of groups
  result += endMultiLine;
  return result;
}

string XHCPThread::commandPutDevConfig( const string& parameter )
{
  writeLog( "XHCPThread::commandPutDevConfig( " + parameter + " )", logLevel::debug );
  static string device;
  if( inMultilineRequest )
  {
    inMultilineRequest = false;
    multilineRequestHandler = 0;
    if( m_deviceManager->storeNewConfig( device, parameter ) )
      return responseCode( 220 ); // storing succeded
    else
      return responseCode( 503 ); // storing failed
  }
  if( !m_deviceManager->contains( parameter ) )
    return responseCode( 417 );

  device = parameter;
  inMultilineRequest = true;
  multilineRequestHandler = &XHCPThread::commandPutDevConfig;
  return responseCode( 320 );
}

string XHCPThread::commandSetGlobal( const string& parameter )
{
  writeLog( "XHCPThread::commandSetGlobal( " + parameter + " )", logLevel::debug );

  vector<string> list;
  split( list, parameter, is_any_of( " " ) );
  if( list.size() != 2 ) return responseCode( 501 );

  xPLCache->updateEntry( list[0], list[1] );
  return responseCode( 232 );
}

string XHCPThread::commandSendXPlMessage( const string& parameter )
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

string XHCPThread::commandQuit( const string& parameter )
{
  writeLog( "XHCPThread::commandQuit( " + parameter + " )", logLevel::debug );
  quit = true;
  return responseCode( 221 );
}
