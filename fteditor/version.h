#ifndef VERSION_H
#define VERSION_H

#define xstr(s)		str(s)
#define str(s)		#s

#define VERSION_MAJOR                           3
#define VERSION_MINOR                           1
#define VERSION_BUILD                           4

#define VER_FILEVERSION           				VERSION_MAJOR,VERSION_MINOR,VERSION_BUILD,0
#define STR_FILEVERSION                     	VERSION_MAJOR##"."##VERSION_MINOR##"."VERSION_BUILD##"."0

#define VER_PRODUCTVERSION						VERSION_MAJOR.VERSION_MINOR.VERSION_BUILD

#define STR_PRODUCTVERSION						xstr(VERSION_MAJOR) "." xstr(VERSION_MINOR) "." xstr(VERSION_BUILD)

#endif // VERSION_H