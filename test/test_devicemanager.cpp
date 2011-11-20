#include "../src/devicemanager.h"
#define BOOST_TEST_MODULE "DeviceManager"
#include <boost/test/unit_test.hpp>

//#include "xplcache.h"
#include "devicemanager.h"

// load globas and give them their space to live
//#include "globals.h"

#if 0
using namespace boost::filesystem;
path xPLHalRootFolder;
path DataFileFolder;
path ScriptEngineFolder;
path rulesFolder;
#endif


class MockXplHandler: public IxPLHandler
{
    public:
        void sendBroadcastMessage( const std::string& msgClass, const std::string& msgType, const xPLMessage::namedValueList& namedValues ) const
        {
            std::cerr << "sendBroadcastMessage(" << msgClass;
            std::cerr << std::endl;
        }

        /** \brief Send a directed message to the xPL network. */
        void sendMessage( const xPL_MessageType type, const std::string& tgtVendor, const std::string& tgtDeviceID, 
                const std::string& tgtInstanceID, const std::string& msgClass, const std::string& msgType, 
                const xPLMessage::namedValueList& namedValues ) const 
        {
            std::cerr << "sendMessage";
            std::cerr << std::endl;
        }

        /** \brief Send a directed message to the xPL network. */
        void sendMessage( const xPL_MessageType type, const std::string& VDI,
                const std::string& msgClass, const std::string& msgType, const xPLMessage::namedValueList& namedValues ) const
        {
            std::cerr << "sendMessage";
            std::cerr << std::endl;
        }
};

class MockXplCache: public IxPLCacheClass
{
    /** \returns true if the element name exists. */
    virtual bool exists( const std::string& name ) const 
    {
         return false; 
    }

    /** \returns the value of element name if it exists - or an empty std::string otherwise. */
    virtual std::string objectValue( const std::string& name ) const { return ""; }

    /** \returns all objectes stored in the cache. */
    virtual std::string listAllObjects( bool forceEverything = false  ) const { return ""; }

    /** \brief Create a new object or update it if it exists in the cache. */
    virtual void updateEntry( const std::string& name, const std::string& value, const bool expires = true ) { }

    /** \brief Delete an object if it exists in the cache - or do nothing. */
    virtual void deleteEntry( const std::string& name ) { }

    /** \brief Delete all expired entries. */
    virtual void flushExpiredEntries( void ) { }

    /** \returns all entries that start with filter. */
    virtual std::vector<std::string> childNodes( const std::string& filter ) const { std::vector<std::string> a; return a; }

    /** \returns all entries that fit the regular expression. */
    virtual std::vector<std::string> filterByRegEx( const boost::regex& regex ) const { std::vector<std::string> a; return a; }

    /** \returns all entries that fit the regular expression. */
    virtual std::vector<std::string> filterByRegEx( const std::string& regex ) const { std::vector<std::string> a; return a; }

    /** \brief Load object cache from file system */
    virtual void loadCache( void ) { }

    /** \brief Save object cache to file system */
    virtual void saveCache( void ) const { }
};

BOOST_AUTO_TEST_SUITE(DeviceManagerSuite);

BOOST_AUTO_TEST_CASE( add_device )
{
    xPL_MessagePtr msg = new xPL_Message;
    msg->messageType = xPL_MESSAGE_TRIGGER;
    msg->hopCount = 0;
    msg->receivedMessage = TRUE; /* TRUE if received, FALSE if being sent */
    msg->sourceVendor = "pnxs";
    msg->sourceDeviceID = "hs485";
    msg->sourceInstanceID = "default1";
    msg->isGroupMessage = FALSE;
    msg->groupName = 0;
    msg->isBroadcastMessage = TRUE;
    msg->targetVendor = "*";
    msg->targetDeviceID = "";
    msg->targetInstanceID = "";
    msg->schemaClass = "blah";
    msg->schemaType = "blub";
    xPL_addMessageNamedValue(msg, "tag", "value");

    MockXplHandler mockHandler;
    MockXplCache mockCache;

    deviceManagerClass dm(&mockHandler, &mockCache);
    BOOST_CHECK( dm.contains("pnxs-hs485.default1") == false);
    dm.processHeartbeat(msg);
    BOOST_CHECK( dm.contains("pnxs-hs485.default1") == true);
}

BOOST_AUTO_TEST_CASE( remove_device )
{
    xPL_MessagePtr msg = new xPL_Message;
    msg->messageType = xPL_MESSAGE_TRIGGER;
    msg->hopCount = 0;
    msg->receivedMessage = TRUE; /* TRUE if received, FALSE if being sent */
    msg->sourceVendor = "pnxs";
    msg->sourceDeviceID = "hs485";
    msg->sourceInstanceID = "default1";
    msg->isGroupMessage = FALSE;
    msg->groupName = 0;
    msg->isBroadcastMessage = TRUE;
    msg->targetVendor = "*";
    msg->targetDeviceID = "";
    msg->targetInstanceID = "";
    msg->schemaClass = "blah";
    msg->schemaType = "blub";
    xPL_addMessageNamedValue(msg, "tag", "value");

    MockXplHandler mockHandler;
    MockXplCache mockCache;

    deviceManagerClass dm(&mockHandler, &mockCache);
    BOOST_CHECK( dm.contains("pnxs-hs485.default1") == false);
    dm.processHeartbeat(msg);
    BOOST_CHECK( dm.contains("pnxs-hs485.default1") == true);

    dm.processRemove(msg);
    BOOST_CHECK( dm.contains("pnxs-hs485.default1") == false);
}

BOOST_AUTO_TEST_SUITE_END();
