/*
rkrsupport - This file is part of the RKWard project. Created: Mon Oct 25 2010
SPDX-FileCopyrightText: 2010-2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKRSUPPORT_H
#define RKRSUPPORT_H

#include <limits.h>

#include <QVariant>
#include <QStringList>

#include "rdata.h"

#include "rkrapi.h"

/** Convenience functions for working with R. */
namespace RKRSupport {
	SEXP callSimpleFun0 (SEXP fun, SEXP env);
	SEXP callSimpleFun (SEXP fun, SEXP arg, SEXP env);
	SEXP callSimpleFun2 (SEXP fun, SEXP arg1, SEXP arg2, SEXP env);
	bool callSimpleBool (SEXP fun, SEXP arg, SEXP env);

	QStringList SEXPToStringList (SEXP from_exp);
	SEXP StringListToSEXP (const QStringList &list);
	SEXP QVariantToSEXP(const QVariant &val);
	QVariant SEXPToNestedStrings(SEXP from_exp);
	QString SEXPToString (SEXP from_exp);
	RData::IntStorage SEXPToIntArray (SEXP from_exp);
	int SEXPToInt (SEXP from_exp, int def_value = INT_MIN);
	RData::RealStorage SEXPToRealArray (SEXP from_exp);
	RData* SEXPToRData (SEXP from_exp);

	/** Replacement for BEGIN_SUSPEND_INTERRUPTS-macro that we cannot easily use */
	class InterruptSuspension {
	public:
		InterruptSuspension() { old_susp = ROb(R_interrupts_suspended); }
		~InterruptSuspension() { ROb(R_interrupts_suspended) = old_susp; if(ROb(R_interrupts_pending)) RFn::Rf_onintr(); }
	private:
		Rboolean old_susp;
	};
};

class RKRShadowEnvironment {
public:
	struct Result {
		QStringList added;
		QStringList removed;
		QStringList changed;
		bool isEmpty() const { return added.isEmpty() && removed.isEmpty() && changed.isEmpty(); };
	};
	Result diffAndUpdate();
	static Result diffAndUpdate(SEXP envir) { return environmentFor(envir)->diffAndUpdate(); };
	static void updateCacheForGlobalenvSymbol(const QString &name);
private:
	RKRShadowEnvironment(SEXP baseenvir, SEXP shadowenvir) : baseenvir(baseenvir), shadowenvir(shadowenvir) {};
	~RKRShadowEnvironment();
	static RKRShadowEnvironment* environmentFor(SEXP baseenvir);
	void updateSymbolCache(const QString &name);
	SEXP baseenvir;
	SEXP shadowenvir;
	static QHash<SEXP, RKRShadowEnvironment*> environments;
	static SEXP shadowenvbase;
};

class RKTextCodec {
public:
	static QString fromNative(const QByteArray& buf) {
		return QString::fromUtf8(doConv(from_native, buf));
	}
	static QByteArray toNative(const QString& buf) {
		return doConv(to_native, buf.toUtf8());
	}
	static void reinit();
private:
	static QByteArray doConv(void *cd, const QByteArray &inp);
	static void *from_native;
	static void *to_native;
};

#endif
