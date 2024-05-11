/*
debug - This file is part of the RKWard project. Created: Sun Aug 8 2004
SPDX-FileCopyrightText: 2004-2017 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#define RKWARD_DEBUG

void RKDebug (int flags, int level, const char *fmt, ...);

// Debug-levels
#define DL_TRACE 0
#define DL_DEBUG 1
#define DL_INFO 2
#define DL_WARNING 3
#define DL_ERROR 4
#define DL_FATAL 5

// Debug components
#define APP 1
#define PLUGIN 1 << 1
#define OBJECTS 1 << 2
#define EDITOR 1 << 3
#define SETTINGS 1 << 4
#define PHP 1 << 5
#define RBACKEND 1 << 6
#define COMMANDEDITOR 1 << 7
#define MISC 1 << 8
#define DIALOGS 1 << 9
#define OUTPUT 1 << 10
#define XML 1 << 11
#define GRAPHICS_DEVICE 1 << 12
#define DEBUG_ALL (APP | PLUGIN | PHP | OBJECTS | EDITOR | SETTINGS | RBACKEND | COMMANDEDITOR | MISC | DIALOGS | OUTPUT | XML | GRAPHICS_DEVICE)

#ifdef RKWARD_DEBUG
// Debug functions 
#	define RK_DO(expr,flags,level) if ((flags & RK_Debug::RK_Debug_Flags) && (level >= RK_Debug::RK_Debug_Level)) { expr; }
#	define RK_DEBUG(flags,level,...) { if ((flags & RK_Debug::RK_Debug_Flags) && (level >= RK_Debug::RK_Debug_Level)) RKDebug (flags,level,__VA_ARGS__); }
#if defined(CPPCHECK_ONLY)
#	define RK_ASSERT(x) assert(x) /* Keep it from complaining. Supressing nullPointerRedundantCheck does not appear to work. */
#else
#	define RK_ASSERT(x) if (!(x)) RK_DEBUG (DEBUG_ALL, DL_FATAL, "Assert '%s' failed at %s - function %s line %d", #x, __FILE__, __FUNCTION__, __LINE__);
#endif
#	ifndef RKWARD_NO_TRACE
#		define RK_TRACE(flags) RK_DEBUG (flags, DL_TRACE, "Trace: %s - function %s line %d", __FILE__, __FUNCTION__, __LINE__);
#	else
#		define RK_TRACE(flags)
#	endif
#else
#	define RK_DO(expr,flags,level)
#	define RK_DEBUG(flags,level,fmt,...)
#	define RK_ASSERT(x)
#	define RK_TRACE(flags)
#endif

class QFile;
namespace RK_Debug {
	extern int RK_Debug_Level;
	extern int RK_Debug_Flags;
	extern int RK_Debug_CommandStep;
	bool setupLogFile (const QString &basename);
	extern QFile* debug_file;
};
