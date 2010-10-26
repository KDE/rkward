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

#include <Rdefines.h>

// needed to detect CHARSXP encoding
#define IS_UTF8(x) (Rf_getCharCE(x) == CE_UTF8)
#define IS_LATIN1(x) (Rf_getCharCE(x) == CE_LATIN1)

#include <QTextCodec>

#include "../rkglobals.h"
#include "rembedinternal.h"
#include "../debug.h"

// This is sort of idiotic, but placing RKWard_RData_Tag into the RKRSupport-namespace somehow confuses the hell out of G++ (4.4.5)
SEXP RKWard_RData_Tag;

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

/** converts SEXP to strings, and returns the first string (or QString(), if SEXP contains no strings) */
QString RKRSupport::SEXPToString (SEXP from_exp) {
	RK_TRACE (RBACKEND);

	QString ret;

	unsigned int count;
	QString *list = SEXPToStringList (from_exp, &count);

	if (count >= 1) ret = list[0];
	delete [] list;
	return ret;
}

QString *RKRSupport::SEXPToStringList (SEXP from_exp, unsigned int *count) {
	RK_TRACE (RBACKEND);

	// bad format? coerce the vector first
	if (TYPEOF (from_exp) != STRSXP) {
		SEXP strexp;
		PROTECT (strexp = Rf_coerceVector (from_exp, STRSXP));
		QString *list = SEXPToStringList (strexp, count);
		UNPROTECT (1);
		return list;
	}

	// format already good? Avoid coercion (and associated copying)
	*count = Rf_length (from_exp);
	QString *list = new QString[*count];
	unsigned int i = 0;
	for (; i < *count; ++i) {
		SEXP dummy = STRING_ELT (from_exp, i);

		if (TYPEOF (dummy) != CHARSXP) {
			list[i] = QString ("not defined");	// can this ever happen?
		} else {
			if (dummy == NA_STRING) {
				list[i] = QString::null;
			} else {
				if (IS_UTF8 (dummy)) {
					list[i] = QString::fromUtf8 ((char *) STRING_PTR (dummy));
				} else if (IS_LATIN1 (dummy)) {
					list[i] = QString::fromLatin1 ((char *) STRING_PTR (dummy));
				} else {
					list[i] = RThread::this_pointer->current_locale_codec->toUnicode ((char *) STRING_PTR (dummy));
				}
			}
		}
	}

	return list;
}

int *RKRSupport::SEXPToIntArray (SEXP from_exp, unsigned int *count) {
	RK_TRACE (RBACKEND);

	int *integers;

	// bad format? coerce the vector first
	if (TYPEOF (from_exp) != INTSXP) {
		SEXP intexp;
		PROTECT (intexp = Rf_coerceVector (from_exp, INTSXP));
		integers = SEXPToIntArray (intexp, count);
		UNPROTECT (1);
		return integers;
	}

	// format already good? Avoid coercion (and associated copying)
	*count = Rf_length (from_exp);
	integers = new int[*count];
	for (unsigned int i = 0; i < *count; ++i) {
		integers[i] = INTEGER (from_exp)[i];
		if (integers[i] == R_NaInt) integers[i] = INT_MIN;		// this has no effect for now, but if R ever chnages it's R_NaInt, then it will
	}
	return integers;
}

/** converts SEXP to integers, and returns the first int (def_value, if SEXP contains no ints) */
int RKRSupport::SEXPToInt (SEXP from_exp, int def_value) {
	RK_TRACE (RBACKEND);

	int ret = def_value;
	unsigned int count;
	int *integers = SEXPToIntArray (from_exp, &count);
	if (count >= 1) ret = integers[0];
	delete [] integers;

	return ret;
}

double *RKRSupport::SEXPToRealArray (SEXP from_exp, unsigned int *count) {
	RK_TRACE (RBACKEND);

	double *reals;

	// bad format? coerce the vector first
	if (TYPEOF (from_exp) != REALSXP) {
		SEXP realexp;
		PROTECT (realexp = Rf_coerceVector (from_exp, REALSXP));
		reals = SEXPToRealArray (realexp, count);
		UNPROTECT (1);
		return reals;
	}
	
	// format already good? Avoid coercion (and associated copying)
	*count = Rf_length (from_exp);
	reals = new double[*count];
	for (unsigned int i = 0; i < *count; ++i) {
		reals[i] = REAL (from_exp)[i];
		if (R_IsNaN (reals[i]) || R_IsNA (reals[i]) ) reals[i] = RKGlobals::na_double;
	}
	return reals;
}

RData *RKRSupport::SEXPToRData (SEXP from_exp) {
	RK_TRACE (RBACKEND);

	RData *data = new RData;

	unsigned int count;
	int type = TYPEOF (from_exp);
	switch (type) {
		case LGLSXP:
		case INTSXP:
			data->data = SEXPToIntArray (from_exp, &count);
			data->datatype = RData::IntVector;
			break;
		case REALSXP:
			data->data = SEXPToRealArray (from_exp, &count);
			data->datatype = RData::RealVector;
			break;
		case VECSXP:
			count = 0;
			count = Rf_length (from_exp);
			{
				RData **structure_array = new RData*[count];
				for (unsigned int i=0; i < count; ++i) {
					SEXP subexp = VECTOR_ELT (from_exp, i);
					//PROTECT (subexp);	// should already be protected as part of the parent from_exp
					structure_array[i] = SEXPToRData (subexp);
					//UNPROTECT (1);
				}
				data->data = structure_array;
			}
			data->datatype = RData::StructureVector;
			break;
/*		case NILSXP:
			data->data = 0;
			data->datatype = RData::NoData;
			count = 0;
			break; */
		case EXTPTRSXP:
			if (R_ExternalPtrTag (from_exp) == RKWard_RData_Tag) {		// our very own data
				delete data;
				data = (RData*) R_ExternalPtrAddr (from_exp);
				R_ClearExternalPtr (from_exp);
				count = data->length;
				break;
			}
		case STRSXP:
		default:
			data->data = SEXPToStringList (from_exp, &count);
			data->datatype = RData::StringVector;
	}

	data->length = count;

	return data;
}
