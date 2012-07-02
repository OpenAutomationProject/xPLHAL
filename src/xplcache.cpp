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
#include <algorithm>

#include "globals.h"
#include "log.h"

#include "xplcache.h"

using boost::algorithm::starts_with;
using boost::algorithm::iequals;
using boost::algorithm::is_any_of;
using boost::algorithm::split;

namespace BFS = boost::filesystem;

using std::string;
using std::vector;
using std::mutex;
using std::lock_guard;


//timeStreamHelper timeConverter;

xPLCacheClass::CacheFilter::CacheFilter() :
    prefTag( "cache" ), mfType( "*" ), mfVendor( "*" ), mfDevice( "*" ),
    mfInstance( "*" ), mfSchemType( "*" ), mfSchemClass( "*" ), expiry( false )
{}

xPLCacheClass::CacheFilter::CacheFilter( const string& filter ) :
    prefTag( "cache" ), expiry( false )
{
    vector<string> list;
    split( list, filter, is_any_of(".") );
    mfType       = list[0];
    mfVendor     = list[1];
    mfDevice     = list[2];
    mfInstance   = list[3];
    mfSchemType  = list[4];
    mfSchemClass = list[5];
}

xPLCacheClass::xPLCacheClass(const boost::filesystem::path& dataFileFolder)
:mDataFileFolder(dataFileFolder)
{
    loadCache();
}

string xPLCacheClass::listAllObjects( bool forceEverything ) const
{
    writeLog( "xPLCacheClass::ListAllObjects()", logLevel::debug );

    string result;
    lock_guard<mutex> locker(mCachelock);

    for( auto &it : mCache ) {
        string name = it.first;
        if( starts_with( name, "device." ) ) {
            if( forceEverything || iequals( "true", mCache.find("xplhal.showdevices")->second.value ) )
                result += name + "=" + it.second.value + "\r\n";
        }
        else if( starts_with( name, "config." ) ) {
            if( forceEverything || iequals( "true", mCache.find("xplhal.showconfigs")->second.value ) )
                result += name + "=" + it.second.value + "\r\n";
        }
        else {
            result += name + "=" + it.second.value + "\r\n";
        }
    }
    writeLog( "shared_lock locker(cacheLock): about to be destructed", logLevel::debug );
    return result;
}

void xPLCacheClass::updateEntry( const string& name, const string& value, const bool expires )
{
    writeLog( "xPLCacheClass::updateEntry( \"" + name + "\", \"" + value + "\" )", logLevel::debug );
    lock_guard<mutex> locker(mCachelock);

    auto it = mCache.find( name );
    if( mCache.end() == it ) {
        mCache.insert({name, CacheEntry( value, expires )});
    }
    else {
        it->second = CacheEntry( value, expires );
    }
}

void xPLCacheClass::deleteEntry( const string& name )
{
    writeLog( "xPLCacheClass::deleteEntry( \"" + name + "\" )", logLevel::debug );
    lock_guard<mutex> locker(mCachelock);
    mCache.erase( name );
}

template<typename Map, typename F>
void map_erase_if(Map& m, F pred)
{
    for (typename Map::iterator i = m.begin();
          (i = std::find_if(i, m.end(), pred)) != m.end();
          m.erase(i++));
}

void xPLCacheClass::flushExpiredEntries( void )
{
    writeLog( "xPLCacheClass::flushExpiredEntries()", logLevel::debug );
    lock_guard<mutex> locker(mCachelock);

    map_erase_if(mCache, [](cachemap_t::value_type &cache) { 
        return cache.second.isExpired();
    });
}

vector<string> xPLCacheClass::childNodes( const string& filter ) const
{
    vector<string> retval;

    for(auto &it : mCache) {
        if( starts_with( it.first, filter ) ) {
            retval.push_back( it.first );
        }
    }

    return retval;
}

vector<string> xPLCacheClass::filterByRegEx( const boost::regex& regex ) const
{
    vector<string> retval;

    for(auto &it : mCache) {
        if( boost::regex_match( it.first, regex ) ) {
            retval.push_back( it.first );
        }
    }

    return retval;
}

void xPLCacheClass::loadCache( void )
{
    mCache.clear();  // erase all entries

    BFS::path objectCacheFile( mDataFileFolder / "object_mCache.xml" );
    writeLog( "loadCache: ["+objectCacheFile.string()+"]", logLevel::debug ); 
    if( BFS::exists( objectCacheFile ) ) {
        writeLog( "Loading cached xPL object from file: "+objectCacheFile.string(), logLevel::debug );
    } 
    else {
        writeLog( "No xPL object cache file found at: "+objectCacheFile.string()+" - creating a new one...", logLevel::debug );
        updateEntry( "xplhal.period"     , "0"   , false ); 
        updateEntry( "xplhal.mode"       , "0"   , false ); 
        updateEntry( "xplhal.showdevices", "true", false ); 
        updateEntry( "xplhal.showconfigs", "true", false ); 
    }
}

void xPLCacheClass::saveCache( void ) const
{}
