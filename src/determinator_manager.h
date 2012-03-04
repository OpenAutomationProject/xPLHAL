#pragma once
#include "determinator.h"
#include <string>
#include <map>
#include <boost/signals2/signal.hpp>
#include "xplhandler.h"

class DeterminatorManager
{
    public:
        DeterminatorManager(const std::string& determinatorDirectory);

        void loadDeterminators();

//        DeterminatorConstPtr getDeterminator(const std::string& guid) const;
        std::string getXmlDeterminator(const std::string& guid) const;
        void storeXmlDeterminator(const std::string& guid, const std::string& xmlDeterminator);
        
    public:
        xPLHandler::signal_t m_sigRceivedXplMessage;

    private:
        std::string mDeterminatorDirectory;
        std::map<std::string, DeterminatorPtr> mDeterminators;
        
};
