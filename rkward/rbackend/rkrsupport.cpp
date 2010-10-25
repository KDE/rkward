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

#include "rkrsupport.h"

#include "../debug.h"

SEXP RKRSupport::callSimpleFun0 (SEXP fun, SEXP env) {
	SEXP call = Rf_allocVector (LANGSXP, 1);
	PROTECT (call);
	SETCAR (call, fun);

	SEXP ret = Rf_eval (call, env);

	UNPROTECT (1); /* call */
	return ret;
}

SEXP RKRSupport::callSimpleFun (SEXP fun, SEXP arg, SEXP env) {
	SEXP call = Rf_allocVector (LANGSXP, 2);
	PROTECT (call);
	SETCAR (call, fun);
	SETCAR (CDR (call), arg);

	SEXP ret = Rf_eval (call, env);

	UNPROTECT (1); /* call */
	return ret;
}

SEXP RKRSupport::callSimpleFun2 (SEXP fun, SEXP arg1, SEXP arg2, SEXP env) {
	SEXP call = Rf_allocVector (LANGSXP, 3);
	PROTECT (call);
	SETCAR (call, fun);
	SETCAR (CDR (call), arg1);
	SETCAR (CDDR (call), arg2);

	SEXP ret = Rf_eval (call, env);

	UNPROTECT (1); /* call */
	return ret;
}

bool RKRSupport::callSimpleBool (SEXP fun, SEXP arg, SEXP env) {
	SEXP res = callSimpleFun (fun, arg, env);
	RK_ASSERT (TYPEOF (res) == LGLSXP);
	return ((bool) LOGICAL (res)[0]);
}
