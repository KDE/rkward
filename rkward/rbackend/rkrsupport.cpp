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

	QStringList list = SEXPToStringList (from_exp);

	if (!list.isEmpty ()) return list[0];
	return QString ();
}

QStringList RKRSupport::SEXPToStringList (SEXP from_exp) {
	RK_TRACE (RBACKEND);

	// bad format? coerce the vector first
	if (TYPEOF (from_exp) != STRSXP) {
		SEXP strexp;
		PROTECT (strexp = Rf_coerceVector (from_exp, STRSXP));
		QStringList list = SEXPToStringList (strexp);
		UNPROTECT (1);
		return list;
	}

	// format already good? Avoid coercion (and associated copying)
	int count = Rf_length (from_exp);
	QStringList list;
#if QT_VERSION >= 0x040700
	list.reserve (count);
#endif
	for (int i = 0; i < count; ++i) {
		SEXP dummy = STRING_ELT (from_exp, i);

		if (TYPEOF (dummy) != CHARSXP) {
			list.append (QString ("not defined"));	// can this ever happen?
		} else {
			if (dummy == NA_STRING) {
				list.append (QString ());
			} else {
				if (IS_UTF8 (dummy)) {
					list.append (QString::fromUtf8 ((char *) STRING_PTR (dummy)));
				} else if (IS_LATIN1 (dummy)) {
					list.append (QString::fromLatin1 ((char *) STRING_PTR (dummy)));
				} else {
					list.append (RKRBackend::this_pointer->current_locale_codec->toUnicode ((char *) STRING_PTR (dummy)));
				}
			}
		}
	}

	return list;
}

RData::IntStorage RKRSupport::SEXPToIntArray (SEXP from_exp) {
	RK_TRACE (RBACKEND);

	RData::IntStorage integers;

	// bad format? coerce the vector first
	if (TYPEOF (from_exp) != INTSXP) {
		SEXP intexp;
		PROTECT (intexp = Rf_coerceVector (from_exp, INTSXP));
		integers = SEXPToIntArray (intexp);
		UNPROTECT (1);
		return integers;
	}

	// format already good? Avoid coercion (and associated copying)
	unsigned int count = Rf_length (from_exp);
	integers.reserve (count);
	for (unsigned int i = 0; i < count; ++i) {
		integers.append (INTEGER (from_exp)[i]);
	}
	return integers;
}

/** converts SEXP to integers, and returns the first int (def_value, if SEXP contains no ints) */
int RKRSupport::SEXPToInt (SEXP from_exp, int def_value) {
	RK_TRACE (RBACKEND);

	RData::IntStorage integers = SEXPToIntArray (from_exp);
	if (!integers.isEmpty ()) return integers[0];
	return def_value;
}

RData::RealStorage RKRSupport::SEXPToRealArray (SEXP from_exp) {
	RK_TRACE (RBACKEND);

	RData::RealStorage reals;

	// bad format? coerce the vector first
	if (TYPEOF (from_exp) != REALSXP) {
		SEXP realexp;
		PROTECT (realexp = Rf_coerceVector (from_exp, REALSXP));
		reals = SEXPToRealArray (realexp);
		UNPROTECT (1);
		return reals;
	}
	
	// format already good? Avoid coercion (and associated copying)
	unsigned int count = Rf_length (from_exp);
	reals.reserve (count);
	for (unsigned int i = 0; i < count; ++i) {
		reals.append (REAL (from_exp)[i]);
		if (R_IsNaN (reals[i]) || R_IsNA (reals[i])) reals[i] = NA_REAL;	// for our purposes, treat all non-numbers as missing
	}
	return reals;
}

RData *RKRSupport::SEXPToRData (SEXP from_exp) {
	RK_TRACE (RBACKEND);

	RData *data = new RData;

	int type = TYPEOF (from_exp);
	switch (type) {
		case LGLSXP:
		case INTSXP:
			data->setData (SEXPToIntArray (from_exp));
			break;
		case REALSXP:
			data->setData (SEXPToRealArray (from_exp));
			break;
		case VECSXP:
			{
				unsigned int count = Rf_length (from_exp);
				RData::RDataStorage structure_array;
				structure_array.reserve (count);
				for (unsigned int i=0; i < count; ++i) {
					SEXP subexp = VECTOR_ELT (from_exp, i);
					//PROTECT (subexp);	// should already be protected as part of the parent from_exp
					structure_array.append (SEXPToRData (subexp));
					//UNPROTECT (1);
				}
				data->setData (structure_array);
			}
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
				break;
			}
		case STRSXP:
		default:
			data->setData (SEXPToStringList (from_exp));
	}

	return data;
}
