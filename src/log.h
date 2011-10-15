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

#ifndef LOG_H
#define LOG_H

#include <iostream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>

using boost::lexical_cast;
using boost::bad_lexical_cast;

#define defaultLogLevel logLevel::debug

// Create UNUSED to show the compiler that that variable might be not used
#if defined(__GNUC__)
#  define UNUSED __attribute__ ((unused))
#else
#  define
#endif

/**
 * \brief The different log levels known.
 */
namespace logLevel
{
  enum logLevel {
    debug,
    all,
    warning,
    error
  };
}

/**
 * \brief The escape sequences to change the text coloring on the terminal.
 */
namespace escape
{
  static const char* Reset     UNUSED = "\033[0m" ;
  static const char* Bold      UNUSED = "\033[1m" ;
  static const char* Blink     UNUSED = "\033[5m" ;
  static const char* Reverse   UNUSED = "\033[7m" ;
  static const char* Normal    UNUSED = "\033[22m";
  static const char* NoBlink   UNUSED = "\033[25m";
  static const char* NoReverse UNUSED = "\033[27m";
  static const char* FgBlack   UNUSED = "\033[30m";
  static const char* FgRed     UNUSED = "\033[31m";
  static const char* FgGreen   UNUSED = "\033[32m";
  static const char* FgBrown   UNUSED = "\033[33m";
  static const char* FgBlue    UNUSED = "\033[34m";
  static const char* FgMagenta UNUSED = "\033[35m";
  static const char* FgCyan    UNUSED = "\033[36m";
  static const char* FgGray    UNUSED = "\033[37m";
  static const char* BgBlack   UNUSED = "\033[40m";
  static const char* BgRed     UNUSED = "\033[41m";
  static const char* BgGreen   UNUSED = "\033[42m";
  static const char* BgBrown   UNUSED = "\033[43m";
  static const char* BgBlue    UNUSED = "\033[44m";
  static const char* BgMagenta UNUSED = "\033[45m";
  static const char* BgCyan    UNUSED = "\033[46m";
  static const char* BgWhite   UNUSED = "\033[47m";
}

/**
 * \brief The base class for logging.
 * This class is helping to simplify the logging mechanism throughout the program.
 */
class logBase
{
    int currentLogLevel;   /**< \brief The log level shown to the user.              */

    bool showLogColor;     /**< \brief Status if colored logging should be used.     */
    bool showLogTimestamp; /**< \brief Status if timestamped logging should be used. */

    void doLog( std::string message, logLevel::logLevel level )
    {
      std::string timestamp( to_simple_string(boost::posix_time::microsec_clock::local_time()) );
      if( level >= currentLogLevel ) 
      {
        if( showLogColor     ) std::cout << escape::FgGray;
        if( showLogTimestamp ) std::cout << "[" << timestamp << "] ";
        if( showLogColor     ) std::cout << escape::Reset;
        std::cout << message << std::endl;
      }
    }

  public:
    logBase() : currentLogLevel( defaultLogLevel ), showLogColor( true ), showLogTimestamp( true ) {}

    /**
     * \brief Show log message.
     * \param message The message.
     * \param level The level to show.
     */
    void operator() ( std::string message, logLevel::logLevel level )
    {
      doLog( message, level );
    }
    void operator() ( const char *message, logLevel::logLevel level )
    {
      std::string m( message );
      doLog( m, level );
    }
} static writeLog;

#endif // LOG_H
