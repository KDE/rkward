/*
rkrsupport - This file is part of RKWard (https://rkward.kde.org). Created: Mon Oct 25 2010
SPDX-FileCopyrightText: 2010-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkrsupport.h"

#include "rkrbackend.h"
#include "../debug.h"

// This is sort of idiotic, but placing RKWard_RData_Tag into the RKRSupport-namespace somehow confuses the hell out of G++ (4.4.5)
SEXP RKWard_RData_Tag;

SEXP RKRSupport::callSimpleFun0 (SEXP fun, SEXP env) {
	SEXP call = RFn::Rf_allocVector (LANGSXP, 1);
	RFn::Rf_protect(call);
	RFn::SETCAR(call, fun);

	SEXP ret = RFn::Rf_eval(call, env);

	RFn::Rf_unprotect (1); /* call */
	return ret;
}

SEXP RKRSupport::callSimpleFun (SEXP fun, SEXP arg, SEXP env) {
	SEXP call = RFn::Rf_allocVector (LANGSXP, 2);
	RFn::Rf_protect(call);
	RFn::SETCAR(call, fun);
	RFn::SETCAR(RFn::CDR(call), arg);

	SEXP ret = RFn::Rf_eval(call, env);

	RFn::Rf_unprotect (1); /* call */
	return ret;
}

SEXP RKRSupport::callSimpleFun2 (SEXP fun, SEXP arg1, SEXP arg2, SEXP env) {
	SEXP call = RFn::Rf_allocVector (LANGSXP, 3);
	RFn::Rf_protect(call);
	RFn::SETCAR(call, fun);
	RFn::SETCAR(RFn::CDR(call), arg1);
	RFn::SETCAR(RFn::CDDR(call), arg2);

	SEXP ret = RFn::Rf_eval(call, env);

	RFn::Rf_unprotect(1); /* call */
	return ret;
}

bool RKRSupport::callSimpleBool (SEXP fun, SEXP arg, SEXP env) {
	SEXP res = callSimpleFun (fun, arg, env);
	if ((RFn::Rf_length (res) < 1) || (RFn::TYPEOF (res) != LGLSXP)) {
		RK_ASSERT (RFn::TYPEOF (res) == LGLSXP);
		RK_ASSERT (RFn::Rf_length (res) >= 1);
		return false;
	}
	return ((bool) RFn::LOGICAL (res)[0]);
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
	if (RFn::TYPEOF (from_exp) != STRSXP) {
		SEXP strexp;
		RFn::Rf_protect (strexp = RFn::Rf_coerceVector (from_exp, STRSXP));
		QStringList list = SEXPToStringList (strexp);
		RFn::Rf_unprotect (1);
		return list;
	}

	// format already good? Avoid coercion (and associated copying)
	int count = RFn::Rf_length (from_exp);
	QStringList list;
	list.reserve (count);
	for (int i = 0; i < count; ++i) {
		SEXP dummy = RFn::STRING_ELT (from_exp, i);

		if (RFn::TYPEOF (dummy) != CHARSXP) {
			list.append (QString ("not defined"));	// can this ever happen?
		} else {
			if (dummy == ROb(R_NaString)) {
				list.append (QString ());
			} else {
				if (IS_UTF8 (dummy)) {
					list.append (QString::fromUtf8 (RFn::R_CHAR(dummy)));
				} else if (IS_LATIN1 (dummy)) {
					list.append (QString::fromLatin1 (RFn::R_CHAR(dummy)));
				} else {
					list.append(RKTextCodec::fromNative(RFn::R_CHAR(dummy)));
				}
			}
		}
	}

	return list;
}

SEXP RKRSupport::StringListToSEXP (const QStringList& list) {
	RK_TRACE (RBACKEND);

	SEXP ret = RFn::Rf_allocVector (STRSXP, list.size ());
	for (int i = 0; i < list.size (); ++i) {
		RFn::SET_STRING_ELT(ret, i, RFn::Rf_mkCharCE(list[i].toUtf8().constData(), CE_UTF8));
	}
	return ret;
}

SEXP RKRSupport::QVariantToSEXP(const QVariant& var) {
	RK_TRACE (RBACKEND);

	if (var.isNull()) return ROb(R_NilValue);

	QMetaType t = var.metaType();
	if (t == QMetaType(QMetaType::Bool)) {
		SEXP ret = RFn::Rf_allocVector(LGLSXP, 1);
		RFn::LOGICAL(ret)[0] = var.toBool();
		return ret;
	} else if (t == QMetaType(QMetaType::Int)) {
		SEXP ret = RFn::Rf_allocVector(INTSXP, 1);
		RFn::INTEGER(ret)[0] = var.toInt();
		return ret;
	} else if (t != QMetaType(QMetaType::QString) && t != QMetaType(QMetaType::QStringList)) {
		RFn::Rf_warning("unsupported QVariant type in QVariantToSEXP");
	}
	QStringList list = var.toStringList();

	SEXP ret = RFn::Rf_allocVector (STRSXP, list.size ());
	for (int i = 0; i < list.size (); ++i) {
		RFn::SET_STRING_ELT(ret, i, RFn::Rf_mkCharCE(list[i].toUtf8().constData(), CE_UTF8));
	}
	return ret;
}

QVariant RKRSupport::SEXPToNestedStrings(SEXP from_exp) {
	RK_TRACE (RBACKEND);

	if (RFn::Rf_isVectorList(from_exp)) {  // NOTE: list() in R is a vectorlist in the C API...
		QVariantList ret;
		for(int i = 0; i < RFn::Rf_length(from_exp); ++i) {
			SEXP el = RFn::VECTOR_ELT(from_exp, i);
			ret.append(SEXPToNestedStrings(el));
		}
		return ret;
	} else if (RFn::Rf_isPairList(from_exp)) {
		QVariantList ret;
		for(SEXP cons = from_exp; cons != ROb(R_NilValue); cons = RFn::CDR(cons)) {
			ret.append(SEXPToNestedStrings(RFn::CAR(cons)));
		}
		return ret;
	}
	return QVariant(SEXPToStringList(from_exp));
}

RData::IntStorage RKRSupport::SEXPToIntArray (SEXP from_exp) {
	RK_TRACE (RBACKEND);

	RData::IntStorage integers;

	// bad format? coerce the vector first
	if (RFn::TYPEOF (from_exp) != INTSXP) {
		SEXP intexp;
		RFn::Rf_protect (intexp = RFn::Rf_coerceVector (from_exp, INTSXP));
		integers = SEXPToIntArray (intexp);
		RFn::Rf_unprotect (1);
		return integers;
	}

	// format already good? Avoid coercion (and associated copying)
	unsigned int count = RFn::Rf_length (from_exp);
	integers.reserve (count);
	for (unsigned int i = 0; i < count; ++i) {
		integers.append (RFn::INTEGER(from_exp)[i]);
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
	if (RFn::TYPEOF (from_exp) != REALSXP) {
		SEXP realexp;
		RFn::Rf_protect (realexp = RFn::Rf_coerceVector (from_exp, REALSXP));
		reals = SEXPToRealArray (realexp);
		RFn::Rf_unprotect (1);
		return reals;
	}
	
	// format already good? Avoid coercion (and associated copying)
	unsigned int count = RFn::Rf_length (from_exp);
	reals.reserve (count);
	for (unsigned int i = 0; i < count; ++i) {
		reals.append (RFn::REAL (from_exp)[i]);
		if (RFn::R_IsNaN(reals[i]) || RFn::R_IsNA(reals[i])) reals[i] = ROb(R_NaReal);	// for our purposes, treat all non-numbers as missing
	}
	return reals;
}

RData *RKRSupport::SEXPToRData (SEXP from_exp) {
	RK_TRACE (RBACKEND);

	RData *data = new RData;

	int type = RFn::TYPEOF (from_exp);
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
				unsigned int count = RFn::Rf_length (from_exp);
				RData::RDataStorage structure_array;
				structure_array.reserve (count);
				for (unsigned int i=0; i < count; ++i) {
					SEXP subexp = RFn::VECTOR_ELT(from_exp, i);
					//RFn::Rf_protect (subexp);	// should already be protected as part of the parent from_exp
					structure_array.append (SEXPToRData (subexp));
					//RFn::Rf_unprotect (1);
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
			if (RFn::R_ExternalPtrTag(from_exp) == RKWard_RData_Tag) {		// our very own data
				delete data;
				data = static_cast<RData*>(RFn::R_ExternalPtrAddr(from_exp));
				RFn::R_ClearExternalPtr(from_exp);
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
	// TODO: probably ROb(R_GlobalEnv) should be special-cased, as this is what we'll check most often (or exclusively?)
	if (!environments.contains(baseenvir)) {
		RK_DEBUG(RBACKEND, DL_DEBUG, "creating new shadow environment for %p\n", baseenvir);
		if (!shadowenvbase) {
			SEXP rkn = RFn::Rf_allocVector(STRSXP, 1);
			RFn::SET_STRING_ELT(rkn, 0, RFn::Rf_mkChar("package:rkward"));
			SEXP rkwardenv = RKRSupport::callSimpleFun(RFn::Rf_install("as.environment"), rkn, ROb(R_GlobalEnv));
			RK_ASSERT(RFn::Rf_isEnvironment(rkwardenv));
			SEXP rkwardvars = RFn::Rf_eval(RFn::Rf_findVar(RFn::Rf_install(".rk.variables"), rkwardenv), ROb(R_BaseEnv));  // NOTE: RFn::Rf_eval to resolve promise
			RK_ASSERT(RFn::Rf_isEnvironment(rkwardvars));
			shadowenvbase = RFn::Rf_findVar(RFn::Rf_install(".rk.shadow.envs"), rkwardvars);
			RK_ASSERT(RFn::Rf_isEnvironment(shadowenvbase));
		}

		char name[sizeof(void*)*2+3];
		sprintf(name, "%p", (void*) baseenvir);
		SEXP tr = RFn::Rf_allocVector(LGLSXP, 1);
		RFn::LOGICAL(tr)[0] = true;
		RFn::Rf_defineVar(RFn::Rf_install(name), RKRSupport::callSimpleFun2(RFn::Rf_install("new.env"), tr, ROb(R_EmptyEnv), ROb(R_GlobalEnv)), shadowenvbase);
		SEXP shadowenvir = RFn::Rf_findVar(RFn::Rf_install(name), shadowenvbase);
		environments.insert(baseenvir, new RKRShadowEnvironment(baseenvir, shadowenvir));
	}
	return environments[baseenvir];
}

void RKRShadowEnvironment::updateCacheForGlobalenvSymbol(const QString& name) {
	RK_DEBUG(RBACKEND, DL_DEBUG, "updating cached value for symbol %s", qPrintable(name));
	environmentFor(ROb(R_GlobalEnv))->updateSymbolCache(name);
}

static void unbindSymbolWrapper(SEXP name, SEXP env) {
#if R_VERSION >= R_Version(4, 0, 0)
	RFn::R_removeVarFromFrame(name, env);
#else
	SEXP call = RFn::Rf_protect(RFn::Rf_allocVector(LANGSXP, 3));
	RFn::SETRFn::CAR(call, RFn::Rf_install("rm"));
	SEXP s = RFn::CDR(call);
	RFn::SETRFn::CAR(s, name);
	s = RFn::CDR(s);
	RFn::SETRFn::CAR(s, env);
	SET_TAG(s, RFn::Rf_install("pos"));
	RFn::Rf_eval(call, ROb(R_BaseEnv));
	RFn::Rf_unprotect(1);
#endif
}

void RKRShadowEnvironment::updateSymbolCache(const QString& name) {
	RK_TRACE(RBACKEND);
	SEXP rname = RFn::Rf_installChar(RFn::Rf_mkCharCE(name.toUtf8().constData(), CE_UTF8));
	RFn::Rf_protect(rname);
	SEXP symbol_g = RFn::Rf_findVar(rname, ROb(R_GlobalEnv));
	RFn::Rf_protect(symbol_g);
	if (symbol_g == ROb(R_UnboundValue)) unbindSymbolWrapper(rname, shadowenvir);
	else RFn::Rf_defineVar(rname, symbol_g, shadowenvir);
	RFn::Rf_unprotect(2);
}

RKRShadowEnvironment::Result RKRShadowEnvironment::diffAndUpdate() {
	RK_TRACE (RBACKEND);
	Result res;

	// find the changed symbols, and copy them to the shadow environment
	SEXP symbols = RFn::R_lsInternal3(baseenvir, TRUE, FALSE);  // envir, all.names, sorted
	RFn::Rf_protect(symbols);
	int count = RFn::Rf_length(symbols);
	for (int i = 0; i < count; ++i) {
		SEXP name = RFn::Rf_installChar(RFn::STRING_ELT(symbols, i));
		RFn::Rf_protect(name);
		SEXP main = RFn::Rf_findVarInFrame(baseenvir, name);
		SEXP cached = RFn::Rf_findVarInFrame(shadowenvir, name);
		if (main != cached) {
			RFn::Rf_defineVar(name, main, shadowenvir);
			if (cached == ROb(R_UnboundValue)) {
				res.added.append(RKRSupport::SEXPToString(name));
			} else {
				res.changed.append(RKRSupport::SEXPToString(name));
			}
		}
		RFn::Rf_unprotect(1);
	}
	RFn::Rf_unprotect(1); // symbols

	// find the symbols only in the shadow environment (those that were removed in the base)
	SEXP symbols2 = RFn::R_lsInternal3(shadowenvir, TRUE, FALSE);
	RFn::Rf_protect(symbols2);
	int count2 = RFn::Rf_length(symbols2);
	if (count != count2) {  // most of the time, no symbols have been removed, so we can skip the expensive check
		for (int i = 0; i < count2; ++i) {
			SEXP name = RFn::Rf_installChar(RFn::STRING_ELT(symbols2, i));
			RFn::Rf_protect(name);
			// NOTE: R_findVar(), here, is enormously faster than searching the result of ls() for the name, at least when there is a large number of symbols.
			// Importantly, environments provided hash-based lookup, by default.
			SEXP main = RFn::Rf_findVarInFrame(baseenvir, name);
			if (main == ROb(R_UnboundValue)) {
				res.removed.append(RKRSupport::SEXPToString(name));
				unbindSymbolWrapper(name, shadowenvir);
				if (++count >= count2) i = count2;  // end loop
			}
			RFn::Rf_unprotect(1);
		}
	}
	RFn::Rf_unprotect(1);  // symbols2

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
		RFn::Riconv(cd, nullptr, nullptr, &outbufpos, &outbytesleft); // init

		RFn::Riconv(cd, &inbufpos, &inbytesleft, &outbufpos, &outbytesleft);
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
	// TODO: Detect if running in a UTF-8 locale, in which case all conversion can be omitted
	if (from_native) {
		RFn::Riconv_close(from_native);
		RFn::Riconv_close(to_native);
	}
	from_native = RFn::Riconv_open("UTF-8", "");
	to_native = RFn::Riconv_open("", "UTF-8");
}
