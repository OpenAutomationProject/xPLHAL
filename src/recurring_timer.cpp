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
#include "recurring_timer.h"
#include <boost/bind.hpp>

RecurringTimer::RecurringTimer(boost::asio::io_service& io_service, 
        const boost::asio::deadline_timer::duration_type& expiry_time, 
        bool startTimer)
:m_timer(io_service, expiry_time)
,m_delay(expiry_time)
,m_running(false) 
{ 
    if (startTimer) {
        start();
    }
}

RecurringTimer::~RecurringTimer() 
{
    stop();
}

void RecurringTimer::start() 
{
    m_running = true;
    m_timer.async_wait(boost::bind(&RecurringTimer::onExpire, this, _1));
}

void RecurringTimer::stop() 
{
    m_running = false;
    m_timer.cancel();
}

void RecurringTimer::onExpire(const boost::system::error_code& e) 
{
    if (m_running) {
        m_timer.expires_at(m_timer.expires_at() + m_delay);
        m_timer.async_wait(boost::bind(&RecurringTimer::onExpire, this, _1));
    }
    sigExpired(e);
}

