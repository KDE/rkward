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
#include "qtscriptbackend.h"

#include <QTimer>
#include <QDir>

#include "klocale.h"
#include "kmessagebox.h"

#include "../misc/rkcommonfunctions.h"

#include "../debug.h"

QtScriptBackend::QtScriptBackend (const QString &filename) : ScriptBackend () {
	RK_TRACE (PHP);

	script_thread = 0;
	QtScriptBackend::filename = filename;
	dead = false;
	busy = true;
}


QtScriptBackend::~QtScriptBackend () {
	RK_TRACE (PHP);

	if (script_thread) script_thread->terminate ();
}

bool QtScriptBackend::initialize (RKComponentPropertyCode *code_property, bool add_headings) {
	RK_TRACE (PHP);

	if (script_thread) {
		RK_DO (qDebug ("another template is already openend in this backend"), PHP, DL_ERROR);
		return false;
	}

	QDir files_path (RKCommonFunctions::getRKWardDataDir () + "phpfiles/");
	QString common_js (files_path.absoluteFilePath ("common.js"));

	script_thread = new QtScriptBackendThread (common_js, filename, this);
	connect (script_thread, SIGNAL (error(const QString&)), this, SLOT (threadError(const QString&)));
	connect (script_thread, SIGNAL (commandDone(const QString&)), this, SLOT (commandDone(const QString&)));
	connect (script_thread, SIGNAL (needData(const QString&)), this, SLOT (needData(const QString&)));
	current_type = ScriptBackend::Ignore;
	script_thread->start ();

	QtScriptBackend::code_property = code_property;
	QtScriptBackend::add_headings = add_headings;

	return true;
}

void QtScriptBackend::destroy () {
	RK_TRACE (PHP);
	if (!dead) {
		dead = true;
		if (script_thread) script_thread->terminate ();
		QTimer::singleShot (10000, this, SLOT (deleteLater()));	// don't wait for ever for the process to die, even if it's somewhat dangerous
	}

	busy = false;
}

void QtScriptBackend::tryNextFunction () {
	RK_TRACE (PHP);

	if (script_thread && (!busy) && (!command_stack.isEmpty ())) {
	/// clean up previous command if applicable
		if (command_stack.first ()->complete) {
			delete command_stack.takeFirst ();
			
			if (!command_stack.count ()) return;
		}
		
		RK_DO (qDebug ("submitting QtScript code: %s", command_stack.first ()->command.toLatin1 ().data ()), PHP, DL_DEBUG);
		script_thread->setCommand (command_stack.first ()->command);
		busy = true;
		command_stack.first ()->complete = true;
		current_flags = command_stack.first ()->flags;
		current_type = command_stack.first ()->type;
	}
}

void QtScriptBackend::writeData (const QString &data) {
	RK_TRACE (PHP);

	RK_DO (qDebug ("submitting data: %s", data.toLatin1 ().data ()), PHP, DL_DEBUG);
	script_thread->setData (data);
	tryNextFunction ();
}

void QtScriptBackend::threadError (const QString &message) {
	RK_TRACE (PHP);

	if (dead) return;	// we are already dead, so we've shown an error before.

	KMessageBox::error (0, i18n ("The QtScript-backend has reported an error:\n%1", message), i18n ("Scripting error"));

	emit (haveError ());
	destroy ();
}

void QtScriptBackend::commandDone (const QString &result) {
	RK_TRACE (PHP);

	commandFinished (result);
}

void QtScriptBackend::needData (const QString &identifier) {
	RK_TRACE (PHP);

	emit (requestValue (identifier));
}


/////////////////////////////////////////////////////////////////////////////////////////

#include <kurl.h>

QtScriptBackendThread::QtScriptBackendThread (const QString &commonfile, const QString &scriptfile, QtScriptBackend *parent) : QThread (parent) {
	RK_TRACE (PHP);

	_commonfile = commonfile;
	_scriptfile = scriptfile;
}

QtScriptBackendThread::~QtScriptBackendThread () {
	RK_TRACE (PHP);
}

void QtScriptBackendThread::setCommand (const QString &command) {
	RK_TRACE (PHP);

	mutex.lock ();
	RK_ASSERT (_command.isNull ());
	if (command.isNull ()) _command = "";
	else _command = command;
	mutex.unlock ();
}

void QtScriptBackendThread::setData (const QString &data) {
	RK_TRACE (PHP);

	mutex.lock ();
	RK_ASSERT (_data.isNull ());
	if (data.isNull ()) _data = "";
	else _data = data;
	mutex.unlock ();
}

QString QtScriptBackendThread::getValue (const QString &identifier) {
	RK_TRACE (PHP);

	emit (needData (identifier));

	QString ret;
	while (1) {
		mutex.lock ();
		if (!_data.isNull ()) {
			ret = _data;
			_data.clear ();
		}
		mutex.unlock ();

		if (!ret.isNull ()) return (ret);

		msleep (5);
	}
}

bool QtScriptBackendThread::includeFile (const QString &filename) {
	RK_TRACE (PHP);
qDebug ("i1");
	QString _filename = filename;
	if (!filename.startsWith ("/")) {
		KUrl script_path = KUrl (QUrl::fromLocalFile (_scriptfile)).upUrl ();
		script_path.addPath (filename);
		_filename = script_path.toLocalFile ();
	}

qDebug ("i2");
        QFile file (_filename);
        if (!file.open (QIODevice::ReadOnly | QIODevice::Text)) {
		emit (error (i18n ("The file \"%1\" (needed by \"%2\") could not be found. Please check your installation.", _filename, _scriptfile)));
		return false;
	}

qDebug ("i3");
	// evaluate in global context
	engine.currentContext ()->setActivationObject (engine.globalObject ());
qDebug ("i4");
	QScriptValue result = engine.evaluate (file.readAll(), _filename);

qDebug ("i5");
	if (result.isError ()) {
qDebug ("i6e");
		QString message = i18n ("File %1, line %2: %3", _filename, engine.uncaughtExceptionLineNumber (), result.toString ());
qDebug (qPrintable (message));
		emit (error (message));
		return false;
	}

qDebug ("i6");
	return true;
}

void QtScriptBackendThread::run () {
	RK_TRACE (PHP);

	QScriptValue backend_object = engine.newQObject (this);
	engine.globalObject ().setProperty ("_RK_backend", backend_object);

qDebug ("a");
	if (!includeFile (_commonfile)) return;
qDebug ("b");
	if (!includeFile (_scriptfile)) return;
qDebug ("c");

	emit (commandDone ("startup complete"));
qDebug ("d");

	QString command;
	while (1) {
		mutex.lock ();
		if (!_command.isNull ()) {
			command = _command;
			_command.clear ();
		}
		mutex.unlock ();

		if (command.isNull ()) {
			msleep (5);
			continue;
		}

		// do it!
qDebug (qPrintable (command));
		QScriptValue result = engine.evaluate (command);
		if (result.isError ()) {
			QString message = result.toString ();
qDebug (qPrintable (message));
			emit (error (message));
			return;
		} else {
			emit (commandDone (result.toString ()));
		}

		command.clear ();
	}
}

#include "qtscriptbackend.moc"
