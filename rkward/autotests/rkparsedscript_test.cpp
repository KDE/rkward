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

#define moveAndCheck(X, Y) moveAndCheckHelper(X, Y, __LINE__)

/** Test RKParsedScript class. */
class RKParsedScriptTest : public QObject {
	Q_OBJECT
  private:
	QString script;
	RKParsedScript ps;

	void loadScript(const QString &relname, bool rmd = false) {
		QFile f(QStringLiteral(TEST_DATA_DIR) + relname);
		bool ok = f.open(QIODevice::ReadOnly | QIODevice::Text);
		QVERIFY(ok);
		script = QString::fromUtf8(f.readAll());
		ps = RKParsedScript(script, rmd);
	}

	void compareScript(RKParsedScript::ContextIndex pos, const QString &expected, int line) {
		auto ctx = ps.getContext(pos);
		const auto found = script.mid(ctx.start, expected.length());
		if (found != expected) {
			qDebug(" -- Real line number of following mismatch is %d -- ", line);
			QCOMPARE(found, expected);
		}
	}

	RKParsedScript::ContextIndex moveAndCheckHelper(const RKParsedScript::ContextIndex newpos, const QString &expected, int line) {
		compareScript(newpos, expected, line);
		return newpos;
	}

	void sanityTestHelper() {
		for (int startpos = 0; startpos < script.length(); ++startpos) {
			QVERIFY(ps.contextAtPos(startpos).valid());
		}
		for (unsigned int i = 0; i < ps.context_list.size(); ++i) {
			RKParsedScript::ContextIndex ctx0(i);
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
			ctx = ps.firstContextInStatement(ctx0); // NOTE: This one may stay at the same position
			ctx = ctx0;
			while (ctx.valid())
				ctx = ps.nextOuter(ctx);
			ctx = ctx0;
			while (ctx.valid())
				ctx = ps.nextToplevel(ctx);
			ctx = ctx0;
			while (ctx.valid())
				ctx = ps.nextStatementOrInner(ctx);
			ctx = ctx0;
			while (ctx.valid())
				ctx = ps.nextCodeChunk(ctx);

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
			ctx = ps.lastContextInStatement(ctx0); // May stay in same position
			ctx = ctx0;
			while (ctx.valid())
				ctx = ps.prevOuter(ctx);
			ctx = ctx0;
			while (ctx.valid())
				ctx = ps.prevToplevel(ctx);
			ctx = ctx0;
			while (ctx.valid())
				ctx = ps.prevStatementOrInner(ctx);
			ctx = ctx0;
			while (ctx.valid())
				ctx = ps.prevCodeChunk(ctx);

			ctx = ctx0;
			while (ctx.valid())
				ctx = ps.parentRegion(ctx);
		}
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
		sanityTestHelper();
	}

	void nextPrevStatement() {
		loadScript(u"script1.R"_s);
		//		ps.serialize();
		auto ctx = ps.contextAtPos(0);
		QVERIFY(ctx.valid());
		ctx = moveAndCheck(ps.nextStatement(ctx), u"Symbol00"_s);
		ctx = moveAndCheck(ps.nextStatement(ctx), u"Symbol01"_s);
		ctx = moveAndCheck(ps.nextStatement(ctx), u"Symbol19"_s);
		ctx = moveAndCheck(ps.nextStatement(ctx), u"Symbol.x"_s);
		ctx = moveAndCheck(ps.nextStatement(ctx), u"FunctionList"_s);
		QVERIFY(!ps.nextStatement(ctx).valid()); // NOTE this need not be set in stone, could e.g. wrap around

		ctx = moveAndCheck(ps.prevStatement(ctx), u"Symbol.x"_s);
		ctx = moveAndCheck(ps.prevStatement(ctx), u"Symbol19"_s);
		ctx = moveAndCheck(ps.prevStatement(ctx), u"Symbol01"_s);
		ctx = moveAndCheck(ps.prevStatement(ctx), u"Symbol00"_s);
		// QVERIFY(!ps.prevStatement(ctx).valid());  // not sure that we want this

		ctx = ps.contextAtPos(script.indexOf(u"Symbol11"));
		ctx = moveAndCheck(ps.nextStatement(ctx), u"Symbol13"_s);
		ctx = moveAndCheck(ps.nextStatement(ctx), u"Symbol15"_s);
		ctx = moveAndCheck(ps.nextStatement(ctx), u"Symbol16"_s);
		ctx = moveAndCheck(ps.nextStatement(ctx), u"Symbol18"_s);
		ctx = moveAndCheck(ps.nextStatement(ctx), u"Symbol19"_s);

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

		const auto symb14 = ps.contextAtPos(script.indexOf(u"Symbol14"));
		ctx = moveAndCheck(ps.prevStatement(symb14), u"Symbol11"_s); // shall stay in parenthesis
		ctx = moveAndCheck(ps.prevStatement(ctx), u"Symbol10"_s);    // shall move out

		ctx = ps.contextAtPos(script.indexOf(u"Argname"));
		ctx = moveAndCheck(ps.nextStatement(ctx), u"makeFunction"_s);
		QVERIFY(!ps.nextStatement(ctx).valid()); // NOTE this need not be set in stone, could e.g. wrap around

		ctx = ps.contextAtPos(script.indexOf(u"{ aaa"));
		ctx = moveAndCheck(ps.nextStatement(ctx), u"{"_s);
		ctx = moveAndCheck(ps.nextStatement(ctx), u"ddd"_s);
		ctx = moveAndCheck(ps.nextStatement(ctx), u"eee"_s);
		ctx = moveAndCheck(ps.nextStatement(ctx), u"jjj"_s);

		const auto symbjjj = ps.contextAtPos(script.indexOf(u"jjj"));
		ctx = moveAndCheck(ps.prevStatement(symbjjj), u"eee"_s);
		ctx = moveAndCheck(ps.prevStatement(ctx), u"ddd"_s);
		ctx = moveAndCheck(ps.prevStatement(ctx), u"{"_s);
		ctx = moveAndCheck(ps.prevStatement(ctx), u"{ aaa"_s);
		ctx = moveAndCheck(ps.prevStatement(ctx), u"makeFunction"_s);
		ctx = moveAndCheck(ps.prevStatement(ctx), u"Argname"_s);
		ctx = moveAndCheck(ps.prevStatement(ctx), u"FunctionList"_s);

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

		auto ctx = ps.contextAtPos(script.indexOf(u"nest3"));
		ctx = moveAndCheck(ps.nextOuter(ctx), u"nest5"_s);
		ctx = moveAndCheck(ps.nextOuter(ctx), u"ddd"_s);
		QVERIFY(!ps.nextOuter(ctx).valid());

		ctx = ps.contextAtPos(script.indexOf(u"nest3"));
		ctx = moveAndCheck(ps.prevOuter(ctx), u"{"_s);
		ctx = moveAndCheck(ps.prevOuter(ctx), u"{"_s);
		ctx = moveAndCheck(ps.prevOuter(ctx), u"makeFunction"_s);
		ctx = moveAndCheck(ps.prevOuter(ctx), u"FunctionList"_s);
		QVERIFY(!ps.prevOuter(ctx).valid());
	}

	void nextPrevToplevel() {
		loadScript(u"script1.R"_s);

		auto ctx = ps.contextAtPos(script.indexOf(u"Symbol09"));
		ctx = moveAndCheck(ps.nextToplevel(ctx), u"Symbol19"_s);
		ctx = moveAndCheck(ps.nextToplevel(ctx), u"Symbol.x"_s);
		ctx = moveAndCheck(ps.nextToplevel(ctx), u"FunctionList"_s);
		QVERIFY(!ps.nextToplevel(ctx).valid());

		ctx = ps.contextAtPos(script.indexOf(u"Symbol09"));
		ctx = moveAndCheck(ps.prevToplevel(ctx), u"Symbol01"_s);
		ctx = moveAndCheck(ps.prevToplevel(ctx), u"Symbol00"_s);
		ctx = moveAndCheck(ps.nextToplevel(ctx), u"Symbol01"_s); // make sure, we can still advance from here
		ctx = moveAndCheck(ps.prevToplevel(ctx), u"Symbol00"_s);
		QVERIFY(!ps.prevToplevel(ctx).valid());
	}

	void nextPrevInner() {
		loadScript(u"script1.R"_s);

		auto ctx = ps.contextAtPos(script.indexOf(u"nest5"));
		ctx = moveAndCheck(ps.nextStatementOrInner(ctx), u"ddd"_s);
		ctx = moveAndCheck(ps.nextStatementOrInner(ctx), u"eee"_s);
		ctx = moveAndCheck(ps.nextStatementOrInner(ctx), u"ggg"_s);
		ctx = moveAndCheck(ps.nextStatementOrInner(ctx), u"jjj"_s);

		ctx = ps.contextAtPos(script.indexOf(u"jjj"));
		ctx = moveAndCheck(ps.prevStatementOrInner(ctx), u"ggg"_s);
		ctx = moveAndCheck(ps.prevStatementOrInner(ctx), u"eee"_s); // TODO: or should it be "fff"?
		ctx = moveAndCheck(ps.prevStatementOrInner(ctx), u"ddd"_s);
		ctx = moveAndCheck(ps.prevStatementOrInner(ctx), u"nest5"_s);
		ctx = moveAndCheck(ps.prevStatementOrInner(ctx), u"nest4"_s);
	}

	void range() {
		loadScript(u"script1.R"_s);

		auto ctx = ps.contextAtPos(script.indexOf(u"Symbol01"));
		QCOMPARE(script.mid(ps.lastPositionInStatement(ctx) + 1, 9), u"\nSymbol19"_s);
		ctx = ps.contextAtPos(script.indexOf(u"Symbol08"));
		QCOMPARE(script.at(ps.lastPositionInStatement(ctx)), u']');
	}

	void rmdTest() {
		loadScript(u"script1.Rmd"_s, true);
		sanityTestHelper();

		auto ctx = ps.contextAtPos(script.indexOf(u".some"));
		QVERIFY(!ps.nextStatement(ctx).valid());

		ctx = ps.contextAtPos(script.indexOf(u"inline"));
		ctx = moveAndCheck(ps.nextStatementOrInner(ctx), u"code)"_s);
		QVERIFY(!ps.nextStatement(ctx).valid());

		ctx = ps.contextAtPos(script.indexOf(u"code)"));
		ctx = moveAndCheck(ps.prevStatement(ctx), u"inline"_s);
		QVERIFY(!ps.prevStatement(ctx).valid());

		ctx = ps.contextAtPos(script.indexOf(u"symb01"));
		ctx = moveAndCheck(ps.nextStatement(ctx), u"symb03"_s);
		ctx = moveAndCheck(ps.nextStatement(ctx), u"symb05"_s);
		ctx = moveAndCheck(ps.nextStatement(ctx), u"symb07"_s);
		QVERIFY(!ps.nextStatement(ctx).valid());

		ctx = ps.contextAtPos(script.indexOf(u"symb14"));
		ctx = moveAndCheck(ps.nextStatement(ctx), u"symb15"_s);
		ctx = moveAndCheck(ps.prevStatement(ctx), u"symb13"_s);
		ctx = moveAndCheck(ps.prevStatement(ctx), u"symb11"_s);
		QVERIFY(!ps.prevStatement(ctx).valid());

		ctx = ps.contextAtPos(script.indexOf(u"This is markdown"));
		ctx = moveAndCheck(ps.nextCodeChunk(ctx), u".some"_s);

		ctx = ps.contextAtPos(script.indexOf(u"and this is too"));
		ctx = moveAndCheck(ps.prevCodeChunk(ctx), u".some"_s);

		ctx = ps.contextAtPos(script.indexOf(u".some"));
		ctx = moveAndCheck(ps.nextCodeChunk(ctx), u"inline"_s);
		ctx = moveAndCheck(ps.nextCodeChunk(ctx), u"symb01"_s);
		ctx = moveAndCheck(ps.nextCodeChunk(ctx), u"symb11"_s);
		QVERIFY(!ps.nextCodeChunk(ctx).valid());

		ctx = ps.contextAtPos(script.indexOf(u" .some"));
		ctx = moveAndCheck(ps.nextCodeChunk(ctx), u"inline"_s);

		ctx = ps.contextAtPos(script.indexOf(u"symb13"));
		ctx = moveAndCheck(ps.prevCodeChunk(ctx), u"symb01"_s);
		ctx = moveAndCheck(ps.prevCodeChunk(ctx), u"inline"_s);
		ctx = moveAndCheck(ps.prevCodeChunk(ctx), u".some"_s);
		QVERIFY(!ps.prevCodeChunk(ctx).valid());
	}
};

QTEST_MAIN(RKParsedScriptTest)

#include "rkparsedscript_test.moc"
