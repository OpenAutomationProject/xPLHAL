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

#ifndef XPLMESSAGEQUEUE_H
#define XPLMESSAGEQUEUE_H

#include <vector>
#include <queue>
#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>

using namespace std;

extern "C" {
#define COMMON_TYPES
typedef bool Bool;
typedef const char * String;
//#include "../../../xPLLib/xPL.h"
#include "xPL.h"
}

/**
 * \brief The C++ version of an xPL message
 */
class xPLMessage
{
  public:
  /** \brief Type that contains all name-value pairs of one xPL message. 
    *  This type has to preserve the order of it's items and must support
    *  multiple entries with the same key. So a map or multimap wan't sufficient.
    */
  typedef std::vector<std::pair<string,string> > namedValueList;

  xPL_MessageType type;
  string vendor;
  string deviceID;
  string instanceID;
  string msgClass;
  string msgType;
  namedValueList namedValues;

  xPLMessage(
    const xPL_MessageType _type,
    const string& _vendor,
    const string& _deviceID,
    const string& _instanceID,
    const string& _msgClass,
    const string& _msgType,
    const namedValueList& _namedValues
  ) : type(_type), vendor(_vendor), deviceID(_deviceID), instanceID(_instanceID),
      msgClass(_msgClass), msgType(_msgType), namedValues(_namedValues) {}
};
typedef boost::shared_ptr<xPLMessage> xPLMessagePtr;

/**
 * Thread save store for xPL messages to send
 */
class xPLMessageQueueClass
{
    /** \brief variable to ensure that the queue is thread save... */
    mutable boost::mutex queueLock;
    typedef boost::lock_guard<boost::mutex> lock_guard;
    queue<xPLMessagePtr> xPLMessages;

  public:
    /** \brief Add an message to the queue that'll be send on the next
               occasion and return a reference to fill it. */
    void add( const xPLMessagePtr& message );

    /** \brief Convert the next message to an xPL message and delete
               it from the queue. The caller has to ensure that the
               xPLLib is currently not called elsewhere, i.e. locked! 
        \returns Returns 0 if no message available or the message. */
    xPL_MessagePtr consume( const xPL_ServicePtr& service );
};

#endif // XPLMESSAGEQUEUE_H
