# try to find the xPL Library

find_path( xPL_INCLUDE_DIR 
  NAMES xPL.h 
  PATHS /usr/local/include
  PATHS /home/thomas/projects/xPLLib
)

find_library( xPL_LIBRARY
  NAMES libxPL.so
  PATHS /usr/local/lib
  PATHS /home/thomas/projects/xPLLib
)

set( xPL_FOUND TRUE )

set( xPL_INCLUDE_DIRS ${xPL_INCLUDE_DIR} )
set( xPL_LIBRARIES ${xPL_LIBRARY} )

message( "Found xPL at ${xPL_LIBRARY} with headers in ${xPL_INCLUDE_DIR}" ) 
