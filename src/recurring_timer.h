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

#include <boost/asio.hpp>

class RecurringTimer
{
    public:
        RecurringTimer(boost::asio::io_service& io_service, const boost::asio::deadline_timer::duration_type& expiry_time, bool startTimer = false);
        ~RecurringTimer();

        void setExpireHandler(void (*handler)(const boost::system::error_code& e));

        void start();
        void stop();

    private:
        void onExpire(const boost::system::error_code& e);

        const boost::asio::deadline_timer::duration_type m_delay;
        bool m_running;
        boost::asio::deadline_timer m_timer; 
        void (*m_expireFunc)(const boost::system::error_code&);
};

