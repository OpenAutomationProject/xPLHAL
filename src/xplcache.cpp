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

#include <boost/algorithm/string.hpp>
#include <boost/thread/locks.hpp>

#include "globals.h"
#include "log.h"

#include "xplcache.h"

using namespace boost::algorithm;
using namespace boost::posix_time;
using namespace boost::filesystem;

timeStreamHelper timeConverter;

xPLCacheClass::CacheFilter::CacheFilter() :
  prefTag( "cache" ), mfType( "*" ), mfVendor( "*" ), mfDevice( "*" ),
  mfInstance( "*" ), mfSchemType( "*" ), mfSchemClass( "*" ), expiry( false )
{}

xPLCacheClass::CacheFilter::CacheFilter( const std::string& filter ) :
  prefTag( "cache" ), expiry( false )
{
  std::vector<std::string> list;
  split( list, filter, is_any_of(".") );
  mfType       = list[0];
  mfVendor     = list[1];
  mfDevice     = list[2];
  mfInstance   = list[3];
  mfSchemType  = list[4];
  mfSchemClass = list[5];
}

xPLCacheClass::xPLCacheClass() 
{
  loadCache();
}

std::string xPLCacheClass::listAllObjects( bool forceEverything ) const
{
  writeLog( "xPLCacheClass::ListAllObjects()", logLevel::debug );

  std::string result;
  shared_lock locker(cacheLock);

  for( cacheMap::const_iterator it = cache.begin(); it != cache.end(); it++ )
  {
    std::string name = it->first;
    if( starts_with( name, "device." ) )
    {
      if( forceEverything || iequals( "true", cache.find("xplhal.showdevices")->second.value ) )
        result += name + "=" + it->second.value + "\r\n";
    }
    else if( starts_with( name, "config." ) )
    {
      if( forceEverything || iequals( "true", cache.find("xplhal.showconfigs")->second.value ) )
        result += name + "=" + it->second.value + "\r\n";
    }
    else
    {
      result += name + "=" + it->second.value + "\r\n";
    }
  }
  writeLog( "shared_lock locker(cacheLock): about to be destructed", logLevel::debug );
  return result;
}

void xPLCacheClass::updateEntry( const std::string& name, const std::string& value, const bool expires )
{
  writeLog( "xPLCacheClass::updateEntry( \"" + name + "\", \"" + value + "\" )", logLevel::debug );
  unique_lock locker(cacheLock);

  cacheMap::iterator it = cache.find( name );
  if( cache.end() == it )
    cache.insert( std::pair<std::string,CacheEntry>( name, CacheEntry( value, expires ) ) );
  else
    it->second = CacheEntry( value, expires );
}

void xPLCacheClass::deleteEntry( const std::string& name )
{
  writeLog( "xPLCacheClass::deleteEntry( \"" + name + "\" )", logLevel::debug );
  unique_lock locker(cacheLock);
  cache.erase( name );
}

void xPLCacheClass::flushExpiredEntries( void )
{
  writeLog( "xPLCacheClass::flushExpiredEntries()", logLevel::debug );
  unique_lock locker(cacheLock);
  for( cacheMap::iterator it = cache.begin(); it != cache.end(); )
  {
    // delete if entry is older than 15 minutes
    if( it->second.expiry && (it->second.date < second_clock::local_time() - minutes( 15 )) )
      cache.erase( it++ );
    else
      ++it;
  }
}

std::vector<std::string> xPLCacheClass::childNodes( const std::string& filter ) const
{
  std::vector<std::string> retval;

  for( cacheMap::const_iterator it = cache.begin(); it != cache.end(); ++it )
    if( starts_with( it->first, filter ) )
      retval.push_back( it->first );

  return retval;
}

std::vector<std::string> xPLCacheClass::filterByRegEx( const boost::regex& regex ) const
{
  std::vector<std::string> retval;

  for( cacheMap::const_iterator it = cache.begin(); it != cache.end(); ++it )
  {
    if( boost::regex_match( it->first, regex ) )
      retval.push_back( it->first );
  }

  return retval;
}

void xPLCacheClass::loadCache( void )
{
  cache.clear();  // erase all entries

  path objectCacheFile( DataFileFolder / "object_cache.xml" );
  writeLog( "loadCache: ["+objectCacheFile.string()+"]", logLevel::debug ); 
  if( boost::filesystem::exists( objectCacheFile ) )
  {
    writeLog( "Loading cached xPL object from file: "+objectCacheFile.string(), logLevel::debug );
  } else {
    writeLog( "No xPL object cache file found at: "+objectCacheFile.string()+" - creating a new one...", logLevel::debug );
    updateEntry( "xplhal.period"     , "0"   , false ); 
    updateEntry( "xplhal.mode"       , "0"   , false ); 
    updateEntry( "xplhal.showdevices", "true", false ); 
    updateEntry( "xplhal.showconfigs", "true", false ); 
  }
}

void xPLCacheClass::saveCache( void ) const
{}
