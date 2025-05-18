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

	void sanityTest() {
		// no matter where we go, and for how long, we shall not crash or hang!
		loadScript(u"script1.R"_s);
		for (int startpos = 0; startpos < script.length(); ++startpos) {
			const auto ctx0 = ps.contextAtPos(script.length() / 2);
			auto ctx = ctx0;
			while (ctx.valid())
				ctx = ps.nextContext(ctx);
			ctx = ctx0;
			while (ctx.valid())
				ctx = ps.nextSibling(ctx);
			ctx = ctx0;
			while (ctx.valid())
				ctx = ps.nextSiblingOrOuter(ctx);
			ctx = ctx0;
			while (ctx.valid())
				ctx = ps.nextStatement(ctx);
			ctx = ctx0;
			while (ctx.valid())
				ctx = ps.prevContext(ctx);
			ctx = ctx0;
			while (ctx.valid())
				ctx = ps.prevSibling(ctx);
			ctx = ctx0;
			while (ctx.valid())
				ctx = ps.prevSiblingOrOuter(ctx);
			ctx = ctx0;
			while (ctx.valid())
				ctx = ps.prevStatement(ctx);
			ctx = ctx0;
			while (ctx.valid())
				ctx = ps.parentRegion(ctx);
		}
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
		QVERIFY(!ps.nextStatement(ctx).valid()); // NOTE this need not be set in stone, could e.g. wrap around

		testLog("Block2");
		ctx = moveAndCheck(ps.prevStatement(ctx), u"Symbol.x"_s);
		ctx = moveAndCheck(ps.prevStatement(ctx), u"Symbol19"_s);
		ctx = moveAndCheck(ps.prevStatement(ctx), u"Symbol01"_s);
		ctx = moveAndCheck(ps.prevStatement(ctx), u"Symbol00"_s);
		// QVERIFY(!ps.prevStatement(ctx).valid());  // not sure that we want this

		testLog("Block3");
		ctx = ps.contextAtPos(script.indexOf(u"Symbol11"));
		ctx = moveAndCheck(ps.nextStatement(ctx), u"Symbol13"_s);
		ctx = moveAndCheck(ps.nextStatement(ctx), u"Symbol15"_s);
		ctx = moveAndCheck(ps.nextStatement(ctx), u"Symbol16"_s);
		ctx = moveAndCheck(ps.nextStatement(ctx), u"Symbol18"_s);
		ctx = moveAndCheck(ps.nextStatement(ctx), u"Symbol19"_s);

		testLog("Block4");
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

		testLog("Block5");
		const auto symb14 = ps.contextAtPos(script.indexOf(u"Symbol14"));
		ctx = moveAndCheck(ps.prevStatement(symb14), u"Symbol11"_s); // shall stay in parenthesis
		ctx = moveAndCheck(ps.prevStatement(ctx), u"Symbol10"_s);    // shall move out

		testLog("Block6");
		ctx = ps.contextAtPos(script.indexOf(u"Argname"));
		ctx = moveAndCheck(ps.nextStatement(ctx), u"makeFunction"_s);
		QVERIFY(!ps.nextStatement(ctx).valid()); // NOTE this need not be set in stone, could e.g. wrap around

		testLog("Block7");
		ctx = ps.contextAtPos(script.indexOf(u"{ aaa"));
		ctx = moveAndCheck(ps.nextStatement(ctx), u"{"_s);
		ctx = moveAndCheck(ps.nextStatement(ctx), u"ddd"_s);
		ctx = moveAndCheck(ps.nextStatement(ctx), u"eee"_s);
		ctx = moveAndCheck(ps.nextStatement(ctx), u"jjj"_s);

		testLog("Block7");
		const auto symbjjj = ps.contextAtPos(script.indexOf(u"jjj"));
		ctx = moveAndCheck(ps.prevStatement(symbjjj), u"eee"_s);
		ctx = moveAndCheck(ps.prevStatement(ctx), u"ddd"_s);
		ctx = moveAndCheck(ps.prevStatement(ctx), u"{"_s);
		ctx = moveAndCheck(ps.prevStatement(ctx), u"{ aaa"_s);
		ctx = moveAndCheck(ps.prevStatement(ctx), u"makeFunction"_s);
		ctx = moveAndCheck(ps.prevStatement(ctx), u"Argname"_s);
		ctx = moveAndCheck(ps.prevStatement(ctx), u"FunctionList"_s);

		testLog("Block8");
		const auto symbnest3 = ps.contextAtPos(script.indexOf(u"nest3"));
		ctx = moveAndCheck(ps.prevStatement(symbnest3), u"nest2"_s);
		ctx = moveAndCheck(ps.prevStatement(ctx), u"{"_s);
		ctx = moveAndCheck(ps.prevStatement(ctx), u"nest1"_s);
		ctx = moveAndCheck(ps.prevStatement(ctx), u"{"_s);
		ctx = moveAndCheck(ps.prevStatement(ctx), u"{ aaa"_s);
		ctx = moveAndCheck(ps.prevStatement(ctx), u"makeFunction"_s);
	}

	void nextPrevOuter() {
		loadScript(u"script1.R"_s);
		testLog("Block1");
		auto ctx = ps.contextAtPos(script.indexOf(u"nest3"));
		ctx = moveAndCheck(ps.nextOuter(ctx), u"nest5"_s);
		ctx = moveAndCheck(ps.nextOuter(ctx), u"ddd"_s);
		QVERIFY(!ps.nextOuter(ctx).valid());

		testLog("Block2");
		ctx = ps.contextAtPos(script.indexOf(u"nest3"));
		ctx = moveAndCheck(ps.prevOuter(ctx), u"{"_s);
		ctx = moveAndCheck(ps.prevOuter(ctx), u"{"_s);
		ctx = moveAndCheck(ps.prevOuter(ctx), u"makeFunction"_s);
		ctx = moveAndCheck(ps.prevOuter(ctx), u"FunctionList"_s);
		QVERIFY(!ps.prevOuter(ctx).valid());
	}

	void nextPrevToplevel() {
		loadScript(u"script1.R"_s);
		testLog("Block1");
		auto ctx = ps.contextAtPos(script.indexOf(u"Symbol09"));
		ctx = moveAndCheck(ps.nextToplevel(ctx), u"Symbol19"_s);
		ctx = moveAndCheck(ps.nextToplevel(ctx), u"Symbol.x"_s);
		ctx = moveAndCheck(ps.nextToplevel(ctx), u"FunctionList"_s);
		QVERIFY(!ps.nextToplevel(ctx).valid());

		testLog("Block2");
		ctx = ps.contextAtPos(script.indexOf(u"Symbol09"));
		ctx = moveAndCheck(ps.prevToplevel(ctx), u"Symbol01"_s);
		ctx = moveAndCheck(ps.prevToplevel(ctx), u"Symbol00"_s);
		ctx = moveAndCheck(ps.nextToplevel(ctx), u"Symbol01"_s); // make sure, we can still advance from here
		ctx = moveAndCheck(ps.prevToplevel(ctx), u"Symbol00"_s);
		QVERIFY(!ps.prevToplevel(ctx).valid());
	}

	void nextPrevInner() {
		loadScript(u"script1.R"_s);
		testLog("Block1");
		auto ctx = ps.contextAtPos(script.indexOf(u"nest5"));
		ctx = moveAndCheck(ps.nextStatementOrInner(ctx), u"ddd"_s);
		ctx = moveAndCheck(ps.nextStatementOrInner(ctx), u"eee"_s);
		ctx = moveAndCheck(ps.nextStatementOrInner(ctx), u"ggg"_s);
		ctx = moveAndCheck(ps.nextStatementOrInner(ctx), u"jjj"_s);

		testLog("Block2");
		ctx = ps.contextAtPos(script.indexOf(u"jjj"));
		ctx = moveAndCheck(ps.prevStatementOrInner(ctx), u"ggg"_s);
		ctx = moveAndCheck(ps.prevStatementOrInner(ctx), u"eee"_s); // TODO: or should it be "fff"?
	}
};

QTEST_MAIN(RKParsedScriptTest)

#include "rkparsedscript_test.moc"
