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
#include <functional>
#include <signal.h>

#include <boost/program_options.hpp>
#include "log.h"
#include "devicemanager.h"
#include "determinator_manager.h"
#include "xhcp.h"
#include "recurring_timer.h"
#include "xplhandler.h"
#include "xplcache.h"

// load globas and give them their space to live
#include "globals.h"

using boost::filesystem::path;
using boost::filesystem::current_path;
using boost::program_options::options_description;
using boost::program_options::variables_map;
using boost::program_options::store;
using boost::program_options::parse_command_line;
using boost::program_options::notify;
using std::cout;
using std::endl;

using namespace std::placeholders;
path xPLHalRootFolder;
path DataFileFolder;
path ScriptEngineFolder;
path rulesFolder;

xPLCacheClass *xPLCache;
xPLHandler *xPL;

static boost::asio::io_service* g_ioservice = nullptr;

/**
 * \brief Main application
 */
class XplHalApplication
{
    public:
        XplHalApplication() 
        :mXplCache(new xPLCacheClass)
        ,mDeviceManager(mXplCache)
        ,mXHCPServer(new XHCPServer(m_ioservice, &mDeviceManager))
        ,mXpl(new xPLHandler(m_ioservice, boost::asio::ip::host_name() ))
        ,mDeterminatorManager("data")
        ,mTimerListAllObjects(m_ioservice,      std::chrono::seconds(60), true)
        ,mTimerFlushExpiredEntries(m_ioservice, std::chrono::minutes(5), true)

        {
            mDeviceManager.m_sigSendXplMessage.connect([&](const xPLMessagePtr ptr) {mXpl->sendMessage(ptr);});
            mXpl->m_sigRceivedXplMessage.connect(std::bind(&DeviceManager::processXplMessage, &mDeviceManager, _1));
            mXpl->m_sigRceivedXplMessage.connect(mDeterminatorManager.m_sigRceivedXplMessage);
            installTimer();

            /* set global variables */
            xPLCache = mXplCache;
            xPL = mXpl;

            writeLog( "initialized", logLevel::all );
        }

        ~XplHalApplication()
        {
            // clean up
            delete mXHCPServer;
            writeLog( "main: xhcp shutdown", logLevel::all );
            //    delete deviceManager;
            //    writeLog( "main: deviceManager shutdown", logLevel::all );
            delete mXplCache;
            writeLog( "main: xPLCache shutdown", logLevel::all );
            delete mXpl;
            writeLog( "main: xPL shutdown", logLevel::all );
        }

        /**
         * \brief Installs timer while constructing
         */
        void installTimer()
        {
            mTimerListAllObjects.sigExpired.connect([](const boost::system::error_code& e) {
                    writeLog( "main: <tick>", logLevel::all );
                    writeLog( "xPLCache:\n" + xPLCache->listAllObjects(), logLevel::debug );
            });

            mTimerFlushExpiredEntries.sigExpired.connect([](const boost::system::error_code& e) {
                    writeLog( "main: flush cache", logLevel::all );
                    xPLCache->flushExpiredEntries(); // flush cache
            });
        }

        /**
         * \brief quit application
         */
        static void stop() 
        {
            m_ioservice.stop();
        }


        /**
         * \brief runs the Application (called from main)
         */
        int exec()
        {
            // force everyone to send their configuration so that we start up to date...
            mXpl->sendMessage(xPLMessagePtr( new xPLMessage(xPL_MESSAGE_COMMAND, "*", "config", "current", {{"command", "request"}}) ));

            writeLog( "started, run mainloop", logLevel::all );
            m_ioservice.run();
            writeLog( "main: shutdown xPLHal", logLevel::all );
            return 0;
        }

        static void dispatchEvent(void (*event)()) 
        {
            m_ioservice.dispatch(event);
        }
        
    private:
        static boost::asio::io_service m_ioservice;

        xPLCacheClass        *mXplCache;
        DeviceManager    mDeviceManager;
        XHCPServer      *mXHCPServer;
        xPLHandler      *mXpl;
        DeterminatorManager mDeterminatorManager;

        RecurringTimer mTimerListAllObjects;
        RecurringTimer mTimerFlushExpiredEntries;
};

boost::asio::io_service XplHalApplication::m_ioservice;

void handle_signal(int signal)
{
    if (signal == SIGINT || signal == SIGTERM) {
        XplHalApplication::stop();
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

    options_description desc("Allowed options");
    desc.add_options()
        ("help", "help message")
        ("datadir","location of data directory");

    variables_map vm;
    store(parse_command_line(argc, argv, desc), vm);
    notify(vm);

    if (vm.count("help")) {
        cout << desc << endl;
        return 1;
    }

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    // FIXME : add exception handling for directory operations!!!
    if(false) {
        //FIXME if( !QDir::setCurrent( xPLHalRootFolder.path() ) )
        writeLog( "Error changing to working directory \"" + xPLHalRootFolder.string() + "\"!", logLevel::error );
        return -1;
    }

    if( !exists( DataFileFolder ) ) {
        writeLog( "Directory \"" + DataFileFolder.string() + "\" for DataFileFolder doesn't exist. Creating it...", logLevel::debug );
        if( !create_directory( DataFileFolder ) ) {
            writeLog( "Error creating data directory \"" + DataFileFolder.string() + "\"!", logLevel::error );
            return -1;
        }
    }

    if( !exists( ScriptEngineFolder ) ) {
        writeLog( "Directory \"" + ScriptEngineFolder.string() + "\" for ScriptEngineFolder doesn't exist. Creating it...", logLevel::debug );
        if( !create_directory( ScriptEngineFolder ) ) {
            writeLog( "Error creating script directory \"" + ScriptEngineFolder.string() + "\"!", logLevel::error );
            return -1;
        }
    }

    if( !exists( rulesFolder ) ) {
        writeLog( "Directory \"" + rulesFolder.string() + "\" for rulesFolder doesn't exist. Creating it...", logLevel::debug );
        if( !create_directory( rulesFolder ) ) {
            writeLog( "Error creating determinator directory \"" + rulesFolder.string() + "\"!", logLevel::error );
            return -1;
        }
    }

    XplHalApplication app;
    return app.exec();
}

