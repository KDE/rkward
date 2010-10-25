/***************************************************************************
                          rkrsupport  -  description
                             -------------------
    begin                : Mon Oct 25 2010
    copyright            : (C) 2010 by Thomas Friedrichsmeier
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

#ifndef RKRSUPPORT_H
#define RKRSUPPORT_H

#include <QString>

#include <Rdefines.h>

/** Convenience functions for working with R. */
namespace RKRSupport {
	SEXP callSimpleFun0 (SEXP fun, SEXP env);
	SEXP callSimpleFun (SEXP fun, SEXP arg, SEXP env);
	SEXP callSimpleFun2 (SEXP fun, SEXP arg1, SEXP arg2, SEXP env);
	bool callSimpleBool (SEXP fun, SEXP arg, SEXP env);
};

#endif