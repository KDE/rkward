/*
qtscriptbackend - This file is part of the RKWard project. Created: Mon Sep 28 2009
SPDX-FileCopyrightText: 2009-2014 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef QTSCRIPTBACKEND_H
#define QTSCRIPTBACKEND_H

#include "scriptbackend.h"

class QtScriptBackendThread;
class RKMessageCatalog;

//#define JSBACKEND_PERFORMANCE_TEST

/** This class allows to use QtScript as a scripting backend in RKWard.

The script itself is run in a separate thread to ensure good performance even for complex scripts. This is especially important for spinboxes, where the value is changes many times in quick succession. Note that this is also the reason not to use Kross, which appears to be not thread safe.

TODO: The code is currently based on the old PHPBackend. Once that is truly obsolete, there should be room for redesigning several aspects. */
class QtScriptBackend : public ScriptBackend {
	Q_OBJECT
public:
	QtScriptBackend (const QString &filename, const RKMessageCatalog *catalog);

	~QtScriptBackend ();

	bool initialize (RKComponentPropertyCode *code_property=nullptr, bool add_headings=true) override;
	void destroy () override;
	
	void preprocess (int flags) override { callFunction ("do_preprocess ();\n", flags, Preprocess); };
	void calculate (int flags) override { callFunction ("do_calculate ();\n", flags, Calculate); };
	void printout (int flags) override { callFunction ("do_printout ();\n", flags, Printout); };
	void preview (int flags) override { callFunction ("do_preview ();\n", flags, Preview); };
	void writeData (const QVariant &data) override;
#ifdef JSBACKEND_PERFORMANCE_TEST
	static void _performanceTest();
#endif
public Q_SLOTS:
	void threadError (const QString &message);
	void commandDone (const QString &result);
	void needData (const QString &identifier, const int hint);
private:
	void tryNextFunction () override;
	QtScriptBackendThread *script_thread;
	const RKMessageCatalog *catalog;

	bool dead;

	QString filename;
};

#include <QThread>
#include <QMutex>
#include <QtQml/QJSEngine>

template<typename T> QJSValue rkJSMakeArray(QJSEngine *engine, QList<T> list) {
	auto ret = engine->newArray(list.size());
	for(int i = 0; i < list.size(); ++i) ret.setProperty(i, list.at(i));
	return ret;
}

class QtScriptBackendThread : public QThread {
	Q_OBJECT
public:
	QtScriptBackendThread (const QString &commonfile, const QString &scriptfile, QtScriptBackend *parent, const RKMessageCatalog *catalog);
	~QtScriptBackendThread ();

	void setCommand (const QString &command);
	void setData (const QVariant &data);
	void kill () { killed = true; };
	void goToSleep (bool sleep);
Q_SIGNALS:
	void commandDone (const QString &result);
	void needData (const QString &identifier, const int hint);
	void error (const QString &error);
protected Q_SLOTS:
	QVariant getValue (const QString &identifier);
	QVariant getList (const QString &identifier);
	QVariant getString (const QString &identifier);
	QVariant getBoolean (const QString &identifier);
	QVariant getUiLabelPair (const QString &identifier);
	bool includeFile (const QString &filename);
protected:
	void run () override;
private:
	/** for any script error in the last evaluation. If there was an error, a message is generated, and this function returns true (and the thread should be made to exit!) */
	bool scriptError(const QJSValue &val);
	QVariant getValue (const QString &identifier, const int hint);

	QString _command;
	QVariant _data;
	QString _commonfile;
	QString _scriptfile;

	QJSEngine engine;
	const RKMessageCatalog *catalog;

	bool killed;

	QMutex mutex;

	QMutex sleep_mutex;
	bool sleeping;
};

#endif
