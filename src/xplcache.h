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

#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <thread>

#include <boost/regex.hpp>
#include "i_xplcache.h"

#ifdef __GXX_EXPERIMENTAL_CXX0X__
#define steady_clock monotonic_clock
#endif

/**
 * \class xPLCacheClass
 *
 * \brief Cache of xPL values and global variables.
 *
 * This class contains a std::map to stores xPL messages and the global variables.
 */
class xPLCacheClass: public IxPLCacheClass
{
    protected:
        /**
         * \brief Representation of a xPL value in the cache. 
         */
        struct CacheEntry
        {
            //! \brief Value of the xPL message.
            std::string value;
            
             //! \brief Timestamp of time of entry.
            std::chrono::time_point<std::chrono::steady_clock> mDate;

            //! \brief Flag if this entry should expire.
            bool   expiry;

            //! \brief Construct the entry with a current timestamp.
            CacheEntry( const std::string& v, bool expires ) : value( v ), mDate(std::chrono::steady_clock::now()), expiry( expires ) {}

            bool isExpired() const {
                return expiry && mDate < (std::chrono::steady_clock::now() - std::chrono::minutes(15));
            }
            
        };

        /** \brief Link a QString as the name to the corresponding value in
         *         the CacheEntry with fast lookup. */
        typedef std::map< std::string, CacheEntry > cachemap_t;
        cachemap_t mCache; /**< \brief The cache itself. */

        /** \brief variable to lock write operations on the cache for safe
         *        multithreading operation. */
        mutable std::mutex mCachelock;

        /**
         * \brief Representation of an xPL filter.
         */
        struct CacheFilter
        {
            std::string prefTag;
            std::string mfType;
            std::string mfVendor;
            std::string mfDevice;
            std::string mfInstance;
            std::string mfSchemType;
            std::string mfSchemClass;
            bool expiry;

            CacheFilter();
            CacheFilter( const std::string& filter );

            /** \returns the xPL filter. */
            std::string xPLFilter( void ) const 
            {
                return mfType + "." + mfVendor + "." + mfDevice + "." + mfInstance + "." + mfSchemType + "." + mfSchemClass;
            }
        };

    public:
        xPLCacheClass();

        /** \returns true if the element name exists. */
        bool exists( const std::string& name ) const { 
            return mCache.end() != mCache.find( name ); 
        }

        /** \returns the value of element name if it exists - or an empty std::string otherwise. */
        std::string objectValue( const std::string& name ) const { 
            auto it = mCache.find( name ); 
            if( mCache.end() == it ) 
                return std::string(); 
            else 
                return it->second.value; 
        }

        /** \returns all objectes stored in the cache. */
        std::string listAllObjects( bool forceEverything = false  ) const;

        /** \brief Create a new object or update it if it exists in the cache. */
        void updateEntry( const std::string& name, const std::string& value, const bool expires = true );

        /** \brief Delete an object if it exists in the cache - or do nothing. */
        void deleteEntry( const std::string& name );

        /** \brief Delete all expired entries. */
        void flushExpiredEntries( void );

        /** \returns all entries that start with filter. */
        std::vector<std::string> childNodes( const std::string& filter ) const;

        /** \returns all entries that fit the regular expression. */
        std::vector<std::string> filterByRegEx( const boost::regex& regex ) const;

        /** \returns all entries that fit the regular expression. */
        std::vector<std::string> filterByRegEx( const std::string& regex ) const { 
            return filterByRegEx( boost::regex( regex ) ); 
        }

        /** \brief Load object cache from file system */
        void loadCache( void );

        /** \brief Save object cache to file system */
        void saveCache( void ) const;
};
