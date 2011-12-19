#include "../src/devicemanager.h"
#define BOOST_TEST_MODULE "DeviceManager"
#include <boost/test/unit_test.hpp>
//#include "mock_xplhandler.h"

//#include "xplcache.h"
#include "devicemanager.h"

#include <boost/regex.hpp>

// load globas and give them their space to live
//#include "globals.h"

#if 0
using namespace boost::filesystem;
path xPLHalRootFolder;
path DataFileFolder;
path ScriptEngineFolder;
path rulesFolder;
#endif


#if 0
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
#endif

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

class MockSignalReceiver
{
    public:
        void handleSignal(const xPLMessagePtr message) 
        {
            std::cout << "received signal" << *message << std::endl;
        }
};

BOOST_AUTO_TEST_SUITE(DeviceManagerSuite);

BOOST_AUTO_TEST_CASE( add_device )
{
    xPLMessage::namedValueList values; 
    values.push_back( std::make_pair( "tag", "value" ) );
    xPLMessagePtr msg( new xPLMessage(xPL_MESSAGE_TRIGGER,
                                      "pnxs", "hs485", "default1",
                                      "blah", "blub",
                                      values) );
#if 0
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
#endif

//    MockXplHandler mockHandler;
    MockXplCache mockCache;

    deviceManagerClass dm(&mockCache);
    BOOST_CHECK( dm.contains("pnxs-hs485.default1") == false);
    dm.processHeartbeat(msg);
    BOOST_CHECK( dm.contains("pnxs-hs485.default1") == true);
}

#define m2s(match, group) std::string(match[group].first, match[group].second)

//xPL_MessagePtr createXplMessage(const char* srcVDI, const char* dstVDI, const char* schema, std::map<std::string, std::string> tags)
xPL_MessagePtr createXplMessage(xPL_MessageType msgType, const std::string& srcVDI, const std::string& dstVDI, const char* schema)
{
    std::string regex = R"(^([\w\d]+)-([\w\d]+)\.([\w\d]+)$)";
    xPL_MessagePtr msg = new xPL_Message;
    msg->messageType = msgType;
    msg->hopCount = 0;
    msg->receivedMessage = TRUE; /* TRUE if received, FALSE if being sent */
    msg->isGroupMessage = FALSE;
    msg->groupName = 0;
    msg->isBroadcastMessage = TRUE;
    msg->schemaClass = "blah";
    msg->schemaType = "blub";
//    xPL_addMessageNamedValue(msg, "tag", "value");

    
    boost::regex re_vdi(regex);
    boost::smatch match;
    if( ! boost::regex_match(srcVDI, match, re_vdi) ) {
        delete msg;
        return 0;
    }
    msg->sourceVendor = m2s(match, 1).c_str();
    msg->sourceDeviceID = m2s(match, 2).c_str();
    msg->sourceInstanceID = m2s(match, 3).c_str();

    if( dstVDI == "*") {
        msg->targetVendor = "*";
        msg->targetDeviceID = "";
        msg->targetInstanceID = "";
    }
    else {
        if( ! boost::regex_match(dstVDI, match, re_vdi) ) {
            delete msg;
            return 0;
        }
    }
    msg->targetVendor = m2s(match, 1).c_str();
    msg->targetDeviceID = m2s(match, 2).c_str();
    msg->targetInstanceID = m2s(match, 3).c_str();

    return 0;
}

BOOST_AUTO_TEST_CASE( remove_device )
{
//    xPL_MessagePtr mymsg = createXplMessage(xPL_MESSAGE_TRIGGER, "pnxs-hs485.default1", "*", "blah.blub");
    xPLMessage::namedValueList values; 
    values.push_back( std::make_pair( "tag", "value" ) );
    xPLMessagePtr msg( new xPLMessage(xPL_MESSAGE_TRIGGER,
                                      "pnxs", "hs485", "default1",
                                      "blah", "blub",
                                      values) );
#if 0
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
#endif

//    MockXplHandler mockHandler;
    MockXplCache mockCache;
    MockSignalReceiver sigrecv;

//    xPLMessage::namedValueList nvl;
//    mockHandler.sendMessage(xPL_MESSAGE_COMMAND, "pnxs", "hs485", "default1", "config", "list", nvl);
//    mockHandler.activate();

    deviceManagerClass dm(&mockCache);
    dm.m_sigSendXplMessage.connect(boost::bind(&MockSignalReceiver::handleSignal, &sigrecv, _1));

    BOOST_CHECK( dm.contains("pnxs-hs485.default1") == false);
    dm.processHeartbeat(msg);
    BOOST_CHECK( dm.contains("pnxs-hs485.default1") == true);

    dm.processRemove(msg);
    BOOST_CHECK( dm.contains("pnxs-hs485.default1") == false);

    //mockHandler.verify();
}

BOOST_AUTO_TEST_SUITE_END();
