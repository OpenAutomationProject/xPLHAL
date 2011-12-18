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

#include <map>
#include <string>

/**
 * \brief Store the response codes.
 */
class responseCodeBase
{
    std::map<int,const char *> codes;
    bool init; /**< \brief true after initial initialisation */

  public:
    responseCodeBase() 
    {
      if(! init )
      {
        codes[201] = "201 Reload successful\r\n";
        codes[202] = "202 \r\n";
        codes[203] = "203 OK\r\n";
        codes[204] = "204 List of settings follow\r\n";
        codes[205] = "205 List of options follow\r\n";
        codes[206] = "206 Setting updated\r\n";
        codes[207] = "207 Error log follows\r\n";
        codes[208] = "208 Requested setting follows\r\n";
        codes[209] = "209 Configuration document follows\r\n";
        codes[210] = "210 Requested script or rule follows\r\n";
        codes[211] = "211 Script saved successfully\r\n";
        codes[212] = "212 List of scripts follows\r\n";
        codes[213] = "213 XPL message transmitted\r\n";
        codes[214] = "214 Script/rule successfully deleted\r\n";
        codes[215] = "215 Configuration document saved\r\n";
        codes[216] = "216 List of XPL devices follows\r\n";
        codes[217] = "217 List of config items follows\r\n";
        codes[218] = "218 List of events follows\r\n";
        codes[219] = "219 Event added successfully\r\n";
        codes[220] = "220 Configuration items received successfully\r\n";
        codes[221] = "221 Closing transmission channel - goodbye.\r\n";
        codes[222] = "222 Event information follows\r\n";
        codes[223] = "223 Event deleted successfully\r\n";
        codes[224] = "224 List of subs follows\r\n";
        codes[225] = "225 Error log cleared\r\n";
        codes[226] = "226 X10 device information updated\r\n";
        codes[227] = "227 X10 device information follows\r\n";
        codes[228] = "228 X10 device deleted\r\n";
        codes[229] = "229 Requested sub follows\r\n";
        codes[230] = "230 Replication mode active\r\n";
        codes[231] = "231 List of global variables follows\r\n";
        codes[232] = "232 Global value updated\r\n";
        codes[233] = "233 Global deleted\r\n";
        codes[234] = "234 Configuration item value(s) follow\r\n";
        codes[235] = "235 Device configuration deleted\r\n";
        codes[237] = "237 List of Determinator Rules follows\r\n";
        codes[238] = "238 Rule added successfully\r\n";
        codes[239] = "239 Statistics follow\r\n";
        codes[240] = "240 List of determinator groups follows\r\n";
        codes[291] = "291 Global value follows\r\n";
        codes[292] = "292 List of x10 device states follows\r\n";
        codes[311] = "311 Enter script, end with <CrLf>.<CrLf>\r\n";
        codes[313] = "313 Send message to be transmitted, end with <CrLf>.<CrLf>\r\n";
        codes[315] = "315 Enter configuration document, end with <CrLf>.<CrLf>\r\n";
        codes[319] = "319 Enter event data, end with <CrLf>.<CrLf>\r\n";
        codes[320] = "320 Enter configuration items, end with <CrLf>.<CrLf>\r\n";
        codes[326] = "326 Enter X10 device information, end with <CrLf>.<CrLf>\r\n";
        codes[338] = "338 Send rule, end with <CrLf>.<CrLf>\r\n";
        codes[401] = "401 Reload failed\r\n";
        codes[403] = "403 Script not executed\r\n";
        codes[405] = "405 No such setting\r\n";
        codes[410] = "410 No such script or rule\r\n";
        codes[416] = "416 No config available for specified device\r\n";
        codes[417] = "417 No such device\r\n";
        codes[418] = "418 No vendor information available for specified device\r\n";
        codes[422] = "422 No such event\r\n";
        codes[429] = "429 No such sub-routine\r\n";
        codes[491] = "491 No such global\r\n";
        codes[500] = "500 Command not recognised\r\n";
        codes[501] = "501 Syntax error\r\n";
        codes[502] = "502 Access denied\r\n";
        codes[503] = "503 Internal error - command not performed\r\n";
        codes[530] = "530 A replication client is already active\r\n";
        codes[600] = "600 Replication data follows\r\n";

        init = true;
      }
    }

/** \returns the message belonging to the code. */
    std::string  operator()( int code ) { return std::string(codes[code]); }
} static responseCode;

