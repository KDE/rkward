/*
core_test - This file is part of RKWard (https://rkward.kde.org). Created: Sun May 18 2025
SPDX-FileCopyrightText: 2025 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QFile>
#include <QStandardPaths>
#include <QTest>

#include "../misc/rkparsedscript.h"

using namespace Qt::Literals::StringLiterals;

void testLog(const char *fmt, va_list args) {
	vprintf(fmt, args);
	printf("\n");
	fflush(stdout);
}

void testLog(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	testLog(fmt, ap);
	va_end(ap);
}

void RKDebug(int, int level, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	testLog(fmt, ap);
	if (level >= DL_ERROR) QFAIL("error message during test (see above)");
	va_end(ap);
}

/** Test RKParsedScript class. */
class RKParsedScriptTest : public QObject {
	Q_OBJECT
  private:
	QString script;
	RKParsedScript ps;

	void loadScript(const QString &relname) {
		QFile f(QStringLiteral(TEST_DATA_DIR) + relname);
		bool ok = f.open(QIODevice::ReadOnly | QIODevice::Text);
		QVERIFY(ok);
		script = QString::fromUtf8(f.readAll());
		ps = RKParsedScript(script);
	}

	void compareScript(RKParsedScript::ContextIndex pos, const QString &expected) {
		auto ctx = ps.getContext(pos);
		QCOMPARE(script.mid(ctx.start, expected.length()), expected);
	}

	RKParsedScript::ContextIndex moveAndCheck(const RKParsedScript::ContextIndex newpos, const QString &expected) {
		compareScript(newpos, expected);
		return newpos;
	}
  private Q_SLOTS:
	void init() {
		testLog("Starting next test");
	}

	void cleanup() {
		testLog("Cleanup");
	}

	void initTestCase() {
		QStandardPaths::setTestModeEnabled(true);
		RK_Debug::RK_Debug_Level = DL_DEBUG;
	}

	void nextPrevStatement() {
		loadScript(u"script1.R"_s);
//		ps.serialize();
		auto ctx = ps.contextAtPos(0);
		QVERIFY(ctx.valid());
		testLog("Block1");
		ctx = moveAndCheck(ps.nextStatement(ctx), u"Symbol00"_s);
		ctx = moveAndCheck(ps.nextStatement(ctx), u"Symbol01"_s);
		ctx = moveAndCheck(ps.nextStatement(ctx), u"Symbol19"_s);
		ctx = moveAndCheck(ps.nextStatement(ctx), u"Symbol.x"_s);
		ctx = moveAndCheck(ps.nextStatement(ctx), u"FunctionList"_s);
		QVERIFY(!ps.nextStatement(ctx).valid()); // NOTE this need not be set in stone

		testLog("Block2");
		ctx = moveAndCheck(ps.prevStatement(ctx), u"Symbol.x"_s);
		ctx = moveAndCheck(ps.prevStatement(ctx), u"Symbol19"_s);
		ctx = moveAndCheck(ps.prevStatement(ctx), u"Symbol01"_s);
		ctx = moveAndCheck(ps.prevStatement(ctx), u"Symbol00"_s);
		//QVERIFY(!ps.prevStatement(ctx).valid());  // not sure that we want this

		testLog("Block3");
		const auto symb18 = ps.contextAtPos(script.indexOf(u"Symbol18"));
		ctx = moveAndCheck(ps.prevStatement(symb18), u"Symbol16"_s);
		ctx = moveAndCheck(ps.prevStatement(ctx), u"Symbol15"_s);
		ctx = moveAndCheck(ps.prevStatement(ctx), u"Symbol10"_s); // shall skip inner
		ctx = moveAndCheck(ps.prevStatement(ctx), u"Symbol08"_s);
		ctx = moveAndCheck(ps.prevStatement(ctx), u"Symbol07"_s);
		ctx = moveAndCheck(ps.prevStatement(ctx), u"for"_s); // shall progressively move out, from here
		ctx = moveAndCheck(ps.prevStatement(ctx), u"function"_s);
		ctx = moveAndCheck(ps.prevStatement(ctx), u"Symbol03"_s);
		ctx = moveAndCheck(ps.prevStatement(ctx), u"Symbol01"_s);

		testLog("Block4");
		const auto symb14 = ps.contextAtPos(script.indexOf(u"Symbol14"));
		ctx = moveAndCheck(ps.prevStatement(symb14), u"Symbol11"_s);  // shall stay in parenthesis
		ctx = moveAndCheck(ps.prevStatement(ctx), u"Symbol10"_s); // shall move out

		testLog("Block5");
		const auto symbjjj = ps.contextAtPos(script.indexOf(u"jjj"));
		ctx = moveAndCheck(ps.prevStatement(symbjjj), u"eee"_s);
		ctx = moveAndCheck(ps.prevStatement(ctx), u"ddd"_s);
		ctx = moveAndCheck(ps.prevStatement(ctx), u"{"_s);
		ctx = moveAndCheck(ps.prevStatement(ctx), u"{ aaa"_s);
		ctx = moveAndCheck(ps.prevStatement(ctx), u"makeFunction"_s);
		ctx = moveAndCheck(ps.prevStatement(ctx), u"Argname"_s);
		ctx = moveAndCheck(ps.prevStatement(ctx), u"FunctionList"_s);

		testLog("Block6");
		const auto symbnest3 = ps.contextAtPos(script.indexOf(u"nest3"));
		ctx = moveAndCheck(ps.prevStatement(symbnest3), u"nest2"_s);
		ctx = moveAndCheck(ps.prevStatement(ctx), u"{"_s);
		ctx = moveAndCheck(ps.prevStatement(ctx), u"nest1"_s);
		ctx = moveAndCheck(ps.prevStatement(ctx), u"{"_s);
		ctx = moveAndCheck(ps.prevStatement(ctx), u"{ aaa"_s);
		ctx = moveAndCheck(ps.prevStatement(ctx), u"makeFunction"_s);
	}
};

QTEST_MAIN(RKParsedScriptTest)

#include "rkparsedscript_test.moc"
