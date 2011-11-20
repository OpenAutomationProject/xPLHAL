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

#ifndef IXPLCACHE_H
#define IXPLCACHE_H

#include <string>
#include <vector>
#include <boost/regex.hpp>

//class boost::regex;

class IxPLCacheClass
{
  public:

    /** \returns true if the element name exists. */
    virtual bool exists( const std::string& name ) const = 0;

    /** \returns the value of element name if it exists - or an empty std::string otherwise. */
    virtual std::string objectValue( const std::string& name ) const = 0;

    /** \returns all objectes stored in the cache. */
    virtual std::string listAllObjects( bool forceEverything = false  ) const = 0;

    /** \brief Create a new object or update it if it exists in the cache. */
    virtual void updateEntry( const std::string& name, const std::string& value, const bool expires = true ) = 0;

    /** \brief Delete an object if it exists in the cache - or do nothing. */
    virtual void deleteEntry( const std::string& name ) = 0;

    /** \brief Delete all expired entries. */
    virtual void flushExpiredEntries( void ) = 0;

    /** \returns all entries that start with filter. */
    virtual std::vector<std::string> childNodes( const std::string& filter ) const = 0;

    /** \returns all entries that fit the regular expression. */
    virtual std::vector<std::string> filterByRegEx( const boost::regex& regex ) const = 0;

    /** \returns all entries that fit the regular expression. */
    virtual std::vector<std::string> filterByRegEx( const std::string& regex ) const = 0;

    /** \brief Load object cache from file system */
    virtual void loadCache( void ) = 0;

    /** \brief Save object cache to file system */
    virtual void saveCache( void ) const = 0;
};

#endif // IXPLCACHE_H
