// config.h --- configuration file                              -*- C++ -*-
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// versioning

#define PROGRAM_MAJOR  0
#define PROGRAM_MINOR  1
#define PROGRAM_BUILD  0

//////////////////////////////////////////////////////////////////////////////
// stringification

#define STRINGIFY(id)   #id
#define XSTRINGIFY(id)  STRINGIFY(id)

//////////////////////////////////////////////////////////////////////////////
// program information

// TODO: Set the program information properly
#define PROGRAM_NAME        "TypicalX11App"
#define PROGRAM_AUTHORS     "Katayama Hirofumi MZ"
#define PROGRAM_YEARS       "2015-2016"

#define VERSION_STRING \
    XSTRINGIFY(PROGRAM_MAJOR) "." \
    XSTRINGIFY(PROGRAM_MINOR) "." \
    XSTRINGIFY(PROGRAM_BUILD)

#define VERSION_INFO_STRING \
    PROGRAM_NAME " " VERSION_STRING " by " PROGRAM_AUTHORS

//////////////////////////////////////////////////////////////////////////////
