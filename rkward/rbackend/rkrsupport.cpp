/*
rkrsupport - This file is part of RKWard (https://rkward.kde.org). Created: Mon Oct 25 2010
SPDX-FileCopyrightText: 2010-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkrsupport.h"

#include <Rdefines.h>
#include <Rversion.h>
#include <R_ext/Riconv.h>

// needed to detect CHARSXP encoding
#define IS_UTF8(x) (Rf_getCharCE(x) == CE_UTF8)
#define IS_LATIN1(x) (Rf_getCharCE(x) == CE_LATIN1)

#include "rkrbackend.h"
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
	if ((Rf_length (res) < 1) || (TYPEOF (res) != LGLSXP)) {
		RK_ASSERT (TYPEOF (res) == LGLSXP);
		RK_ASSERT (Rf_length (res) >= 1);
		return false;
	}
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
	list.reserve (count);
	for (int i = 0; i < count; ++i) {
		SEXP dummy = STRING_ELT (from_exp, i);

		if (TYPEOF (dummy) != CHARSXP) {
			list.append (QString ("not defined"));	// can this ever happen?
		} else {
			if (dummy == NA_STRING) {
				list.append (QString ());
			} else {
				if (IS_UTF8 (dummy)) {
					list.append (QString::fromUtf8 (CHAR (dummy)));
				} else if (IS_LATIN1 (dummy)) {
					list.append (QString::fromLatin1 (CHAR (dummy)));
				} else {
					list.append(RKTextCodec::fromNative(CHAR(dummy)));
				}
			}
		}
	}

	return list;
}

SEXP RKRSupport::StringListToSEXP (const QStringList& list) {
	RK_TRACE (RBACKEND);

	SEXP ret = Rf_allocVector (STRSXP, list.size ());
	for (int i = 0; i < list.size (); ++i) {
		SET_STRING_ELT(ret, i, Rf_mkCharCE(list[i].toUtf8().constData(), CE_UTF8));
	}
	return ret;
}

SEXP RKRSupport::QVariantToSEXP(const QVariant& var) {
	RK_TRACE (RBACKEND);

	if (var.isNull()) return R_NilValue;

	QMetaType t = var.metaType();
	if (t == QMetaType(QMetaType::Bool)) {
		SEXP ret = Rf_allocVector(LGLSXP, 1);
		LOGICAL(ret)[0] = var.toBool();
		return ret;
	} else if (t == QMetaType(QMetaType::Int)) {
		SEXP ret = Rf_allocVector(INTSXP, 1);
		INTEGER(ret)[0] = var.toInt();
		return ret;
	} else if (t != QMetaType(QMetaType::QString) && t != QMetaType(QMetaType::QStringList)) {
		Rf_warning("unsupported QVariant type in QVariantToSEXP");
	}
	QStringList list = var.toStringList();

	SEXP ret = Rf_allocVector (STRSXP, list.size ());
	for (int i = 0; i < list.size (); ++i) {
		SET_STRING_ELT(ret, i, Rf_mkCharCE(list[i].toUtf8().constData(), CE_UTF8));
	}
	return ret;
}

QVariant RKRSupport::SEXPToNestedStrings(SEXP from_exp) {
	RK_TRACE (RBACKEND);

	if (Rf_isVectorList(from_exp)) {  // NOTE: list() in R is a vectorlist in the C API...
		QVariantList ret;
		for(int i = 0; i < Rf_length(from_exp); ++i) {
			SEXP el = VECTOR_ELT(from_exp, i);
			ret.append(SEXPToNestedStrings(el));
		}
		return ret;
	} else if (Rf_isPairList(from_exp)) {
		QVariantList ret;
		for(SEXP cons = from_exp; cons != R_NilValue; cons = CDR(cons)) {
			ret.append(SEXPToNestedStrings(CAR(cons)));
		}
		return ret;
	}
	return QVariant(SEXPToStringList(from_exp));
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
#if (QT_VERSION >= QT_VERSION_CHECK(5,8,0))
		Q_FALLTHROUGH();
#endif
		//case STRSXP: // intentional fallthrough, conversion to stringlist is the default handling
		default:
			data->setData (SEXPToStringList (from_exp));
	}

	return data;
}

SEXP RKRShadowEnvironment::shadowenvbase = nullptr;
QHash<SEXP, RKRShadowEnvironment*> RKRShadowEnvironment::environments;
RKRShadowEnvironment* RKRShadowEnvironment::environmentFor(SEXP baseenvir) {
	RK_TRACE(RBACKEND);
	// TODO: probably R_GlobalEnv should be special-cased, as this is what we'll check most often (or exclusively?)
	if (!environments.contains(baseenvir)) {
		RK_DEBUG(RBACKEND, DL_DEBUG, "creating new shadow environment for %p\n", baseenvir);
		if (!shadowenvbase) {
			SEXP rkn = Rf_allocVector(STRSXP, 1);
			SET_STRING_ELT(rkn, 0, Rf_mkChar("package:rkward"));
			SEXP rkwardenv = RKRSupport::callSimpleFun(Rf_install("as.environment"), rkn, R_GlobalEnv);
			RK_ASSERT(Rf_isEnvironment(rkwardenv));
			SEXP rkwardvars = Rf_eval(Rf_findVar(Rf_install(".rk.variables"), rkwardenv), R_BaseEnv);  // NOTE: Rf_eval to resolve promise
			RK_ASSERT(Rf_isEnvironment(rkwardvars));
			shadowenvbase = Rf_findVar(Rf_install(".rk.shadow.envs"), rkwardvars);
			RK_ASSERT(Rf_isEnvironment(shadowenvbase));
		}

		char name[sizeof(void*)*2+3];
		sprintf(name, "%p", (void*) baseenvir);
		SEXP tr = Rf_allocVector(LGLSXP, 1);
		LOGICAL(tr)[0] = true;
		Rf_defineVar(Rf_install(name), RKRSupport::callSimpleFun2(Rf_install("new.env"), tr, R_EmptyEnv, R_GlobalEnv), shadowenvbase);
		SEXP shadowenvir = Rf_findVar(Rf_install(name), shadowenvbase);
		environments.insert(baseenvir, new RKRShadowEnvironment(baseenvir, shadowenvir));
	}
	return environments[baseenvir];
}

void RKRShadowEnvironment::updateCacheForGlobalenvSymbol(const QString& name) {
	RK_DEBUG(RBACKEND, DL_DEBUG, "updating cached value for symbol %s", qPrintable(name));
	environmentFor(R_GlobalEnv)->updateSymbolCache(name);
}

static void unbindSymbolWrapper(SEXP name, SEXP env) {
#if R_VERSION >= R_Version(4, 0, 0)
	R_removeVarFromFrame(name, env);
#else
	SEXP call = PROTECT(Rf_allocVector(LANGSXP, 3));
	SETCAR(call, Rf_install("rm"));
	SEXP s = CDR(call);
	SETCAR(s, name);
	s = CDR(s);
	SETCAR(s, env);
	SET_TAG(s, Rf_install("pos"));
	Rf_eval(call, R_BaseEnv);
	UNPROTECT(1);
#endif
}

void RKRShadowEnvironment::updateSymbolCache(const QString& name) {
	RK_TRACE(RBACKEND);
	SEXP rname = Rf_installChar(Rf_mkCharCE(name.toUtf8().constData(), CE_UTF8));
	PROTECT(rname);
	SEXP symbol_g = Rf_findVar(rname, R_GlobalEnv);
	PROTECT(symbol_g);
	if (symbol_g == R_UnboundValue) unbindSymbolWrapper(rname, shadowenvir);
	else Rf_defineVar(rname, symbol_g, shadowenvir);
	UNPROTECT(2);
}

RKRShadowEnvironment::Result RKRShadowEnvironment::diffAndUpdate() {
	RK_TRACE (RBACKEND);
	Result res;

	// find the changed symbols, and copy them to the shadow environment
	SEXP symbols = R_lsInternal3(baseenvir, TRUE, FALSE);  // envir, all.names, sorted
	PROTECT(symbols);
	int count = Rf_length(symbols);
	for (int i = 0; i < count; ++i) {
		SEXP name = Rf_installChar(STRING_ELT(symbols, i));
		PROTECT(name);
		SEXP main = Rf_findVarInFrame(baseenvir, name);
		SEXP cached = Rf_findVarInFrame(shadowenvir, name);
		if (main != cached) {
			Rf_defineVar(name, main, shadowenvir);
			if (cached == R_UnboundValue) {
				res.added.append(RKRSupport::SEXPToString(name));
			} else {
				res.changed.append(RKRSupport::SEXPToString(name));
			}
		}
		UNPROTECT(1);
	}
	UNPROTECT(1); // symbols

	// find the symbols only in the shadow environment (those that were removed in the base)
	SEXP symbols2 = R_lsInternal3(shadowenvir, TRUE, FALSE);
	PROTECT(symbols2);
	int count2 = Rf_length(symbols2);
	if (count != count2) {  // most of the time, no symbols have been removed, so we can skip the expensive check
		for (int i = 0; i < count2; ++i) {
			SEXP name = Rf_installChar(STRING_ELT(symbols2, i));
			PROTECT(name);
			// NOTE: R_findVar(), here, is enormously faster than searching the result of ls() for the name, at least when there is a large number of symbols.
			// Importantly, environments provided hash-based lookup, by default.
			SEXP main = Rf_findVarInFrame(baseenvir, name);
			if (main == R_UnboundValue) {
				res.removed.append(RKRSupport::SEXPToString(name));
				unbindSymbolWrapper(name, shadowenvir);
				if (++count >= count2) i = count2;  // end loop
			}
			UNPROTECT(1);
		}
	}
	UNPROTECT(1);  // symbols2

	RK_DEBUG(RBACKEND, DL_DEBUG, "added %s\n", qPrintable(res.added.join(", ")));
	RK_DEBUG(RBACKEND, DL_DEBUG, "changed %s\n", qPrintable(res.changed.join(", ")));
	RK_DEBUG(RBACKEND, DL_DEBUG, "removed %s\n", qPrintable(res.removed.join(", ")));
	return res;
}

QByteArray RKTextCodec::doConv(void *cd, const QByteArray &inp) {
	const char *inbuf = inp.constData();
	size_t inbytesleft = inp.size();
	const char *inbufpos = inbuf;
	char outbuf[8192];
	QByteArray ret;
	while (inbytesleft) {
		char *outbufpos = outbuf;
		size_t outbytesleft = 8192;
		Riconv(cd, nullptr, nullptr, &outbufpos, &outbytesleft); // init

		Riconv(cd, &inbufpos, &inbytesleft, &outbufpos, &outbytesleft);
		ret += QByteArray(outbuf, 8192-outbytesleft);
// Do we need 0 termination?
		if (!inbytesleft) return ret; // done

		if (outbytesleft > 100) {
			// conversion failed but the output buffer still has plenty of room ->
			// we must have encountered an invalid / incomplete multibyte char in inbuf. Let's try next char.
			ret.append(*inbufpos);
			inbufpos++;
			inbytesleft--;
		} // NOTE else: outbuf buffer wasn't lage enough: we just loop
	}
	return ret;
}

void *RKTextCodec::from_native = nullptr;
void *RKTextCodec::to_native = nullptr;
void RKTextCodec::reinit() {
	if (from_native) {
		Riconv_close(from_native);
		Riconv_close(to_native);
	}
	from_native = Riconv_open("UTF-8", "");
	to_native = Riconv_open("", "UTF-8");
}
