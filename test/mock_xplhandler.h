#include <mockpp/visiting/CountedVisitableMethod.h>
#include "i_xplhandler.h"

using namespace mockpp;

class MockXplHandler : public IxPLHandler, public VisitableMockObject
{
    public:
        MockXplHandler()
            : VisitableMockObject("MockXplHandler", 0)
            , sendBroadcastMessage_mocker("sendBroadcastMessage", this)
            , sendMessage_mocker("sendMessage", this)
            , sendMessage2_mocker("sendMessage", this)
        {}

        void sendBroadcastMessage( const std::string& msgClass, const std::string& msgType, const xPLMessage::namedValueList& namedValues ) const 
        {
#if 0
            std::cout << "sendBroadcastMessage(";
            std::cout << msgClass << ",";
            std::cout << msgType << ",";
            std::cout << std::endl;
#endif
            sendBroadcastMessage_mocker.forward(msgClass, msgType);
        }
//        VisitableMockMethod<void, std::string, xPLMessage::namedValueList> sendBroadcastMessage_mocker;
        VisitableMockMethod<void, std::string, std::string> sendBroadcastMessage_mocker;

        /** \brief Send a directed message to the xPL network. */
        void sendMessage( const xPL_MessageType type, const std::string& tgtVendor, const std::string& tgtDeviceID, 
                const std::string& tgtInstanceID, const std::string& msgClass, const std::string& msgType, 
                const xPLMessage::namedValueList& namedValues ) const
        {
#if 0
            std::cout << "sendMessage(";
            std::cout << type << ",";
            std::cout << tgtVendor << "-" << tgtDeviceID << "." << tgtInstanceID << ",";
            std::cout << msgClass << "," << msgType;
            std::cout << std::endl;
#endif
            sendMessage_mocker.forward(type, tgtVendor, tgtDeviceID, tgtInstanceID, msgClass, msgType);
        }
//        VisitableMockMethod<void, std::string, std::string, std::string, std::string, std::string, xPLMessage::namedValueList> sendMessage_mocker;
        VisitableMockMethod<void, xPL_MessageType, std::string, std::string, std::string, std::string, std::string> sendMessage_mocker;

        /** \brief Send a directed message to the xPL network. */
        void sendMessage( const xPL_MessageType type, const std::string& VDI,
                const std::string& msgClass, const std::string& msgType, const xPLMessage::namedValueList& namedValues ) const
        {
#if 0
            std::cout << "sendMessage2(";
            std::cout << type << ",";
            std::cout << VDI << ",";
            std::cout << msgClass << "," << msgType;
            std::cout << std::endl;
#endif
            sendMessage2_mocker.forward(type, VDI, msgClass, msgType);
        }
//        VisitableMockMethod<void, std::string, std::string, std::string, xPLMessage::namedValueList> sendMessage2_mocker;
        VisitableMockMethod<void, xPL_MessageType, std::string, std::string, std::string> sendMessage2_mocker;
};
    
