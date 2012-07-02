#pragma once
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

#define BOOST_BIND_NO_PLACEHOLDERS
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/signals2/signal.hpp>

/**
 * \brief Self-repeating timer
 * Based on the ASIO deadline_timer which only expires once this class
 * implements a recurring timer.
 */
class RecurringTimer
{
    public:
        RecurringTimer(boost::asio::io_service& io_service, const boost::asio::steady_timer::duration& expiry_time, bool startTimer = false);
        ~RecurringTimer();

        void start();
        void stop();
        
        /**
         * \brief Expired signal to connect to
         * This member is public to make it more easy to connect to signal.
         * So only connect/disconnect to this signal is allowed!
         */
        boost::signals2::signal<void (const boost::system::error_code& e)> sigExpired;

    private:
        void onExpire(const boost::system::error_code& e);

        const boost::asio::steady_timer::duration m_delay;
        bool m_running;
        boost::asio::steady_timer m_timer; 
};

