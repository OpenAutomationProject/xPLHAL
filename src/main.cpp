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

#include <signal.h>
#include "log.h"
#include "xplcache.h"
#include "devicemanager.h"
#include "xhcp.h"

// load globas and give them their space to live
#include "globals.h"
using namespace boost::filesystem;

path xPLHalRootFolder;
path DataFileFolder;
path ScriptEngineFolder;
path rulesFolder;

xPLCacheClass *xPLCache;
deviceManagerClass *deviceManager;
xPLHandler *xPL;
xPLMessageQueueClass *xPLMessageQueue;

static boost::condition_variable g_exit_condition;
static boost::mutex g_exit_mutex;
static bool g_exit = false;

void handle_signal(int signal)
{
    if (signal == SIGINT || signal == SIGTERM) {
        g_exit_mutex.lock();
        g_exit = true;
        g_exit_condition.notify_one();
        g_exit_mutex.unlock();
    }
}

/**
 * Setup the whole program.
 */
int main(int UNUSED argc, char** UNUSED argv)
{
    /** Make sure the necessary infrastructure exists: */
    xPLHalRootFolder   = initial_path();
    DataFileFolder     = xPLHalRootFolder / "data";
    ScriptEngineFolder = DataFileFolder   / "scripts";
    rulesFolder        = DataFileFolder   / "determinator";
    //vendorFileFolder   = DataFileFolder / "vendors";
    //ConfigFileFolder   = DataFileFolder / "configs";

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    // FIXME : add exception handling for directory operations!!!
    if(false)//FIXME if( !QDir::setCurrent( xPLHalRootFolder.path() ) )
    {
        writeLog( "Error changing to working directory \"" + xPLHalRootFolder.string() + "\"!", logLevel::error );
        return -1;
    }

    if( !exists( DataFileFolder ) )
    {
        writeLog( "Directory \"" + DataFileFolder.string() + "\" for DataFileFolder doesn't exist. Creating it...", logLevel::debug );
        if( !create_directory( DataFileFolder ) )
        {
            writeLog( "Error creating data directory \"" + DataFileFolder.string() + "\"!", logLevel::error );
            return -1;
        }
    }

    if( !exists( ScriptEngineFolder ) )
    {
        writeLog( "Directory \"" + ScriptEngineFolder.string() + "\" for ScriptEngineFolder doesn't exist. Creating it...", logLevel::debug );
        if( !create_directory( ScriptEngineFolder ) )
        {
            writeLog( "Error creating script directory \"" + ScriptEngineFolder.string() + "\"!", logLevel::error );
            return -1;
        }
    }

    if( !exists( rulesFolder ) )
    {
        writeLog( "Directory \"" + rulesFolder.string() + "\" for rulesFolder doesn't exist. Creating it...", logLevel::debug );
        if( !create_directory( rulesFolder ) )
        {
            writeLog( "Error creating determinator directory \"" + rulesFolder.string() + "\"!", logLevel::error );
            return -1;
        }
    }

    xPLMessageQueue = new xPLMessageQueueClass;
    xPLCache = new xPLCacheClass;
    XHCPServer *xhcpServer = new XHCPServer();
    xPL = new xPLHandler( boost::asio::ip::host_name() ); //xPL->start();
    deviceManager = new deviceManagerClass(xPLCache);
    xPL->connect(boost::bind(&deviceManagerClass::processXplMessage, deviceManager, _1));
    writeLog( "started", logLevel::all );

    // force everyone to send their configuration so that we start up to date...
    xPLMessage::namedValueList command_request; command_request.push_back( std::make_pair( "command", "request" ) );
    xPL->sendBroadcastMessage( "config", "list", command_request );

    // main loop
    for( int count = 0;; ++count )
    {
        {
            boost::mutex::scoped_lock lock(g_exit_mutex);
            if (g_exit) break;
            boost::system_time const wait_until = boost::get_system_time() + boost::posix_time::seconds(10);
            g_exit_condition.timed_wait(lock, wait_until);
            if (g_exit) break;
        }
       
        writeLog( "main: run events", logLevel::all ); // run events

        if( 0 == count%6 )
        {
            writeLog( "main: <tick>", logLevel::all );
            writeLog( "xPLCache:\n" + xPLCache->listAllObjects(), logLevel::debug );
        }
        if( 30 == count ) // every 5 minutes
        {
            writeLog( "main: flush cache", logLevel::all );
            xPLCache->flushExpiredEntries(); // flush cache
            count = 0;
        }
    }
        
    writeLog( "main: shutdown xPLHal", logLevel::all );

    // clean up
    delete xhcpServer;
    writeLog( "main: xhcp shutdown", logLevel::all );
    delete deviceManager;
    writeLog( "main: deviceManager shutdown", logLevel::all );
    delete xPLCache;
    writeLog( "main: xPLCache shutdown", logLevel::all );
    delete xPL;
    writeLog( "main: xPL shutdown", logLevel::all );
}
