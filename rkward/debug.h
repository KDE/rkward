/***************************************************************************
                          debug  -  description
                             -------------------
    begin                : Sun Aug 8 2004
    copyright            : (C) 2004, 2006 by Thomas Friedrichsmeier
    email                : tfry@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#define RKWARD_DEBUG

extern int RK_Debug_Level;
extern int RK_Debug_Flags;
extern int RK_Debug_CommandStep;
extern void RKDebug (int flags, int level, const char *fmt, ...);

// Debug-levels
#define DL_TRACE 0
#define DL_DEBUG 1
#define DL_INFO 2
#define DL_WARNING 3
#define DL_ERROR 4
#define DL_FATAL 5

// Debug components
#define APP 1
#define PLUGIN 2
#define OBJECTS 4
#define EDITOR 8
#define SETTINGS 16
#define PHP 64
#define RBACKEND 128
#define COMMANDEDITOR 256
#define MISC 512
#define DIALOGS 1024
#define OUTPUT 2048
#define XML 4096
#define ALL (APP | PLUGIN | PHP | OBJECTS | EDITOR | RBACKEND | COMMANDEDITOR | MISC | DIALOGS | OUTPUT | XML)

#ifdef RKWARD_DEBUG
// Debug functions 
#	define RK_DO(expr,flags,level) if ((flags & RK_Debug_Flags) && (level >= RK_Debug_Level)) { expr; }
#	define RK_DEBUG(flags,level,...) { if ((flags & RK_Debug_Flags) && (level >= RK_Debug_Level)) RKDebug (flags,level,__VA_ARGS__); }
#	define RK_ASSERT(x) if (!(x)) RKDebug (ALL, DL_FATAL, "Assert failed at %s - function %s line %d", __FILE__, __FUNCTION__, __LINE__);
#	ifndef RKWARD_NO_TRACE
#		define RK_TRACE(flags) RKDebug (flags, DL_TRACE, "Trace: %s - function %s line %d", __FILE__, __FUNCTION__, __LINE__);
#	else
#		define RK_TRACE(flags)
#	endif
#else
#	define RK_DO(expr,flags,level)
#	define RK_DEBUG(flags,level,fmt,...)
#	define RK_ASSERT(x)
#	define RK_TRACE(flags)
#endif
