/***************************************************************************
                          debug  -  description
                             -------------------
    begin                : Sun Aug 8 2004
    copyright            : (C) 2004 by Thomas Friedrichsmeier
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

#ifdef RKWARD_DEBUG

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
#define PHP 64
#define ALL (APP | PLUGIN | PHP)

// only for now
#define RK_DEBUG_FLAGS ALL
#define RK_DEBUG_LEVEL DL_TRACE

// Debug functions 
#define RK_DO(expr,flags,level) if ((flags | RK_DEBUG_FLAGS) && (level >= RK_DEBUG_LEVEL)) { expr; }
#define RK_ASSERT(x) if (!(x)) qDebug ("Assert failed at %s - function %s line %d", __FILE__, __FUNCTION__, __LINE__);
#define RK_TRACE(flags) RK_DO (qDebug ("Trace: %s - function %s line %d", __FILE__, __FUNCTION__, __LINE__), flags, DL_TRACE); 

#else

#define RK_ASSERT

#endif
