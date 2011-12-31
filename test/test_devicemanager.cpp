#include "../src/devicemanager.h"
#define BOOST_TEST_MODULE "DeviceManager"
#include <boost/test/unit_test.hpp>
#include <mockpp/visiting/CountedVisitableMethod.h>

using namespace mockpp;

#include "devicemanager.h"

#include <boost/regex.hpp>

xPLMessagePtr createXplMessage(xPL_MessageType type, const std::string& srcVDI, const std::string& dstVDI, 
                             const std::string& msgClass, const std::string& msgType, xPLMessage::namedValueList list)
{
    xPLMessagePtr msg( new xPLMessage(type, srcVDI,
                                         msgClass, msgType,
                                         list));
    msg->setTargetFromVDI(dstVDI);
    return msg;
}

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

class MockSignalReceiver : public VisitableMockObject
{
    public:
        MockSignalReceiver()
        :VisitableMockObject("MockSignalReceiver", 0)
        ,handleSignal_mocker("handleSignal", this)
        {}

        virtual void handleSignal(const xPLMessagePtr message) 
        {
            std::cout << "received signal" << *message << std::endl;
            handleSignal_mocker.forward(*message);
        }
        VisitableMockMethod<void, xPLMessage> handleSignal_mocker;
};

BOOST_AUTO_TEST_SUITE(DeviceManagerSuite);

BOOST_AUTO_TEST_CASE( add_device )
{
    xPLMessagePtr msg( createXplMessage(xPL_MESSAGE_TRIGGER, "pnxs-hs485.default1", "*", "blah", "blub", {{"tag", "value"}}));
    MockXplCache mockCache;

    deviceManagerClass dm(&mockCache);
    BOOST_CHECK( dm.contains("pnxs-hs485.default1") == false);
    dm.processHeartbeat(msg);
    BOOST_CHECK( dm.contains("pnxs-hs485.default1") == true);
}


BOOST_AUTO_TEST_CASE( remove_device )
{
    xPLMessagePtr msg( createXplMessage(xPL_MESSAGE_TRIGGER, "pnxs-hs485.default1", "*", "blah", "blub", {{"tag", "value"}}));

    MockXplCache mockCache;
    MockSignalReceiver sigrecv;

    sigrecv.handleSignal(createXplMessage(xPL_MESSAGE_COMMAND, "pnxs-hs485.default1", "*", "config", "list", {{"command", "request"}}));
    sigrecv.activate();

    deviceManagerClass dm(&mockCache);
    dm.m_sigSendXplMessage.connect(boost::bind(&MockSignalReceiver::handleSignal, &sigrecv, _1));

    BOOST_CHECK( dm.contains("pnxs-hs485.default1") == false);
    dm.processHeartbeat(msg);
    BOOST_CHECK( dm.contains("pnxs-hs485.default1") == true);

    dm.processRemove(msg);
    BOOST_CHECK( dm.contains("pnxs-hs485.default1") == false);

    sigrecv.verify();
}

BOOST_AUTO_TEST_SUITE_END();
