
/***************************************************************************
                          rembedinternal  -  description
                             -------------------
    begin                : Sun Jul 25 2004
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

extern "C" {

#include "Rinternals.h"
#include "R.h"

#include <stdio.h>

#include "rembedinternal.h"

REmbedInternal::REmbedInternal(){
}

REmbedInternal::~REmbedInternal(){
}

void REmbedInternal::shutdown () {
}

bool REmbedInternal::startR (const char* r_home) {
	extern int Rf_initEmbeddedR(int argc, char **argv);
	setenv("R_HOME", r_home, 1);
	char *argv[] = {"--gui=none", "--slave", "--no-save"};
	int argc = sizeof(argv)/sizeof(argv[0]);
	if (Rf_initEmbeddedR(argc, argv) < 0) {
		return false;
	}
	
	runCommandInternal ("options (pager=\"xless\")\n");
	runCommandInternal ("sink (\"r_out\")\n");
	runCommandInternal ("sink (file (\"r_err\", \"w\"), FALSE, \"message\")\n");
}

bool REmbedInternal::runCommandInternal (const char *command) {
// heavy copying from RServe below
	int maxParts=1;
	int r_error = 0;
	int stat;
	int *status = &stat;
	const char *c = command;
	SEXP cv, pr;

	while (*c) {
		if (*c=='\n' || *c==';') maxParts++;
		c++;
	}

	PROTECT(cv=allocVector(STRSXP, 1));
	SET_VECTOR_ELT(cv, 0, mkChar(command));  

	while (maxParts>0) {
		extern SEXP R_ParseVector(SEXP, int, int*);
		pr=R_ParseVector(cv, maxParts, status);
		// 2=incomplete; 4=eof
		if (*status!=2 && *status!=4) {
			r_error = 1;
			break;
		}
		maxParts--;
	}
	UNPROTECT(1);

	if (*status == 1) {
		PROTECT (pr);
		SEXP exp;
		int errorOccurred;

		exp=R_NilValue;
		if (TYPEOF(pr)==EXPRSXP && LENGTH(pr)>0) {
			int bi=0;
			while (bi<LENGTH(pr)) {
				SEXP pxp=VECTOR_ELT(pr, bi);
				r_error=0;
				exp=R_tryEval(pxp, R_GlobalEnv, &r_error);
				bi++;
				if (r_error) {
					break;
				}
			}
		} else {
			r_error=0;
			exp=R_tryEval(pr, R_GlobalEnv, &r_error);
		}

		UNPROTECT(1); /* pr */
	}
	
	return (r_error != 0);
}

} // extern "C"
