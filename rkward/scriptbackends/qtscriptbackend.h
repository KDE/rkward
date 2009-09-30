/***************************************************************************
                          krossbackend  -  description
                             -------------------
    begin                : Mon Sep 28 2009
    copyright            : (C) 2009 by Thomas Friedrichsmeier
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

#ifndef QTSCRIPTBACKEND_H
#define QTSCRIPTBACKEND_H

#include "scriptbackend.h"

class QtScriptBackendThread;

/** This class allows to use QtScript as a scripting backend in RKWard.

The script itself is run in a separate thread to ensure good performance even for complex scripts. This is especially important for spinboxes, where the value is changes many times in quick succession. Note that this is also the reason not to use Kross, which appears to be not thread safe.

TODO: The code is currently based on the old PHPBackend. Once that is truely obsolete, there should be room for redesigning several aspects. */
class QtScriptBackend : public ScriptBackend {
	Q_OBJECT
public:
	QtScriptBackend (const QString &filename);

	~QtScriptBackend ();

	bool initialize (RKComponentPropertyCode *code_property=0, bool add_headings=true);
	void destroy ();
	
	void preprocess (int flags) { callFunction ("do_preprocess ();\n", flags, Preprocess); };
	void calculate (int flags) { callFunction ("do_calculate ();\n", flags, Calculate); };
	void printout (int flags) { callFunction ("do_printout ();\n", flags, Printout); };
	void preview (int flags) { callFunction ("do_preview ();\n", flags, Preview); };
	void writeData (const QString &data);
public slots:
	void threadError (const QString &message);
	void commandDone (const QString &result);
	void needData (const QString &identifier);
private:
	void tryNextFunction ();
	QtScriptBackendThread *script_thread;

	bool dead;

	QString filename;
};

#include <QThread>
#include <QMutex>
#include <QScriptEngine>

class QtScriptBackendThread : public QThread {
	Q_OBJECT
public:
	QtScriptBackendThread (const QString &commonfile, const QString &scriptfile, QtScriptBackend *parent);
	~QtScriptBackendThread ();

	void setCommand (const QString &command);
	void setData (const QString &data);
signals:
	void commandDone (const QString &result);
	void needData (const QString &identifier);
	void error (const QString &error);
protected slots:
	QString getValue (const QString &identifier);
	bool includeFile (const QString &filename);
protected:
	void run ();
private:
	QString _command;
	QString _data;
	QString _commonfile;
	QString _scriptfile;

	QScriptEngine engine;

	QMutex mutex;
};

#endif
