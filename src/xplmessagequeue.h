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

#include "xplmessage.h"

/**
 * Thread save store for xPL messages to send
 */
class xPLMessageQueueClass
{
    /** \brief variable to ensure that the queue is thread save... */
    mutable boost::mutex queueLock;
    typedef boost::lock_guard<boost::mutex> lock_guard;
    std::queue<xPLMessagePtr> xPLMessages;

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
