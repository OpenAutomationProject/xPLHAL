#pragma once
#include "determinator.h"
#include <string>
#include <map>
#include <boost/signals2/signal.hpp>
#include "xplhandler.h"

/**
 * \brief Manages Determinators in a filesystem directory
 */
class DeterminatorManager
{
    public:
        DeterminatorManager(const std::string& determinatorDirectory);

        /** \brief load Determinators from disk */
        void loadDeterminators();

//        DeterminatorConstPtr getDeterminator(const std::string& guid) const;

        /** \brief get Determinator with GUID guid as std::string */
        std::string getXmlDeterminator(const std::string& guid) const;

        /** \brief store a XML-Formatted Determinator with guid on disk */
        void storeXmlDeterminator(const std::string& guid, const std::string& xmlDeterminator);
        
    public:
        xPLHandler::signal_t m_sigRceivedXplMessage;

    private:
        std::string mDeterminatorDirectory;
        std::map<std::string, DeterminatorPtr> mDeterminators;
        
};
