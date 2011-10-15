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

#include "log.h"

#include "xplmessagequeue.h"

void xPLMessageQueueClass::add( const xPLMessagePtr& message )
{
  writeLog("xPLMessageQueueClass::add", logLevel::debug);
  lock_guard locker( queueLock ); // get exclusive access to the queue
  xPLMessages.push( message );
}

xPL_MessagePtr xPLMessageQueueClass::consume( const xPL_ServicePtr& service )
{
  xPLMessagePtr message;
  {
    lock_guard locker( queueLock );   // get exclusive access to the queue
    if( xPLMessages.empty() )
      return 0;
    message = xPLMessages.front();    // and release is as soon as possible
    xPLMessages.pop();
  }
  writeLog("xPLMessageQueueClass::consume", logLevel::debug);

  xPL_MessagePtr theMessage = 0;
  if( "*" == message->vendor )
  {
    theMessage = xPL_createBroadcastMessage( service, message->type );
  } else {
    theMessage = xPL_createTargetedMessage( service, 
      message->type, const_cast<char*>(message->vendor.c_str()),
      const_cast<char*>(message->deviceID.c_str()), const_cast<char*>(message->instanceID.c_str()) 
    );
  }
  if ( theMessage == NULL )
  {
    writeLog("Unable to create targeted message", logLevel::debug);
  }

  /* Install the schema */
  writeLog("Install the schema", logLevel::debug);
  xPL_setSchema( theMessage, const_cast<char*>(message->msgClass.c_str()), const_cast<char*>(message->msgType.c_str()) );

  for( xPLMessage::namedValueList::const_iterator it = message->namedValues.begin(); it != message->namedValues.end(); it++ )
  {
    xPL_addMessageNamedValue( theMessage, const_cast<char*>(it->first.c_str()), const_cast<char*>(it->second.c_str()) );
  }
  writeLog("namedValues added", logLevel::debug);

  return theMessage;
}


