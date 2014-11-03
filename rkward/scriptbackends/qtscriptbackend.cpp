/***************************************************************************
                          qtscriptbackend  -  description
                             -------------------
    begin                : Mon Sep 28 2009
    copyright            : (C) 2009, 2010, 2012 by Thomas Friedrichsmeier
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
#include "qtscripti18n.h"

#include "../debug.h"

QtScriptBackend::QtScriptBackend (const QString &filename, const RKMessageCatalog *catalog) : ScriptBackend () {
	RK_TRACE (PHP);

	script_thread = 0;
	QtScriptBackend::filename = filename;
	QtScriptBackend::catalog = catalog;
	dead = false;
	busy = true;
}

QtScriptBackend::~QtScriptBackend () {
	RK_TRACE (PHP);

	destroy ();
	if (script_thread && script_thread->isRunning ()) script_thread->terminate ();
}

bool QtScriptBackend::initialize (RKComponentPropertyCode *code_property, bool add_headings) {
	RK_TRACE (PHP);

	if (script_thread) {
		RK_DEBUG (PHP, DL_ERROR, "another template is already openend in this backend");
		return false;
	}

	QDir files_path (RKCommonFunctions::getRKWardDataDir () + "phpfiles/");
	QString common_js (files_path.absoluteFilePath ("common.js"));

	script_thread = new QtScriptBackendThread (common_js, filename, this, catalog);
	connect (script_thread, SIGNAL (error(const QString&)), this, SLOT (threadError(const QString&)));
	connect (script_thread, SIGNAL (commandDone(const QString&)), this, SLOT (commandDone(const QString&)));
	connect (script_thread, SIGNAL (needData(const QString&, const int)), this, SLOT (needData(const QString&, const int)));
	current_type = ScriptBackend::Ignore;
	script_thread->start ();

	QtScriptBackend::code_property = code_property;
	QtScriptBackend::add_headings = add_headings;

	return true;
}

void QtScriptBackend::destroy () {
	RK_TRACE (PHP);
	if (!dead) {
		if (script_thread) script_thread->goToSleep (false);
		dead = true;
		code_property = 0;
		if (script_thread) script_thread->kill ();
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
			
			if (command_stack.isEmpty ()) {
				script_thread->goToSleep (true);
				return;
			}
		}
		
		RK_DEBUG (PHP, DL_DEBUG, "submitting QtScript code: %s", command_stack.first ()->command.toLatin1 ().data ());
		if (script_thread) script_thread->goToSleep (false);
		script_thread->setCommand (command_stack.first ()->command);
		busy = true;
		command_stack.first ()->complete = true;
		current_flags = command_stack.first ()->flags;
		current_type = command_stack.first ()->type;
	} else {
		if (script_thread && command_stack.isEmpty ()) script_thread->goToSleep (true);
	}
}

void QtScriptBackend::writeData (const QVariant &data) {
	RK_TRACE (PHP);

	RK_DEBUG (PHP, DL_DEBUG, "submitting data: %s", qPrintable (data.toString ()));
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

void QtScriptBackend::needData (const QString &identifier, const int hint) {
	RK_TRACE (PHP);

	emit (requestValue (identifier, hint));
}


/////////////////////////////////////////////////////////////////////////////////////////

#include <kurl.h>

QtScriptBackendThread::QtScriptBackendThread (const QString &commonfile, const QString &scriptfile, QtScriptBackend *parent, const RKMessageCatalog *catalog) : QThread (parent), engine (0) {
	RK_TRACE (PHP);

	// you'd think the engine already was in this thread, but no, it is not. You'd also think, this was fixable by setting "this" as the engine's parent, instead of 0, but no, somehow not.
	engine.moveToThread (this);
	_commonfile = commonfile;
	_scriptfile = scriptfile;
	killed = false;
	sleeping = false;
	QtScriptBackendThread::catalog = catalog;
}

QtScriptBackendThread::~QtScriptBackendThread () {
	RK_TRACE (PHP);
}

void QtScriptBackendThread::goToSleep (bool sleep) {
	RK_TRACE (PHP);
	if (sleeping != sleep) {
		if (sleep) {
			sleep_mutex.lock ();	// hold a mutex until it's time to wake up, again. Thread will then wait on this mutex.
			sleeping = true;
		} else {
			sleeping = false;
			sleep_mutex.unlock ();
		}
	}
}

void QtScriptBackendThread::setCommand (const QString &command) {
	RK_TRACE (PHP);

	mutex.lock ();
	RK_ASSERT (_command.isNull ());
	if (command.isNull ()) _command = "";
	else _command = command;
	mutex.unlock ();
}

void QtScriptBackendThread::setData (const QVariant &data) {
	RK_TRACE (PHP);

	mutex.lock ();
	RK_ASSERT (_data.isNull ());
	if (data.isNull ()) _data = "";
	else _data = data;
	mutex.unlock ();
}

QVariant QtScriptBackendThread::getValue (const QString &identifier, const int hint) {
	RK_TRACE (PHP);

	emit (needData (identifier, hint));

	QVariant ret;
	while (1) {
		if (killed) return QVariant ();

		mutex.lock ();
		if (!_data.isNull ()) {
			ret = _data;
			_data.clear ();
		}
		mutex.unlock ();

		if (!ret.isNull ()) break;

		usleep (20);	// getValue () may be called very often, and we expect an answer very soon, so we don't sleep too long.
	}
	return (ret);
}

QVariant QtScriptBackendThread::getValue (const QString &identifier) {
	return getValue (identifier, RKStandardComponent::TraditionalValue);
}

QVariant QtScriptBackendThread::getList (const QString &identifier) {
	return getValue (identifier, RKStandardComponent::StringlistValue);
}

QVariant QtScriptBackendThread::getString (const QString &identifier) {
	return getValue (identifier, RKStandardComponent::StringValue);
}

QVariant QtScriptBackendThread::getBoolean (const QString &identifier) {
	return getValue (identifier, RKStandardComponent::BooleanValue);
}

bool QtScriptBackendThread::scriptError () {
	RK_TRACE (PHP);

	if (!engine.hasUncaughtException ()) return false;

	QString message = i18n ("Script Error: %1\nBacktrace:\n%2", engine.uncaughtException ().toString (), engine.uncaughtExceptionBacktrace ().join ("\n"));
	engine.clearExceptions ();
	emit (error (message));

	return true;
}

bool QtScriptBackendThread::includeFile (const QString &filename) {
	RK_TRACE (PHP);

	QString _filename = filename;
	if (QFileInfo (filename).isRelative ()) {
		KUrl script_path = KUrl (QUrl::fromLocalFile (_scriptfile)).upUrl ();
		script_path.addPath (filename);
		_filename = script_path.toLocalFile ();
	}

		QFile file (_filename);
		if (!file.open (QIODevice::ReadOnly | QIODevice::Text)) {
		emit (error (i18n ("The file \"%1\" (needed by \"%2\") could not be found. Please check your installation.", _filename, _scriptfile)));
		return false;
	}

	// evaluate in global context
	engine.currentContext ()->setActivationObject (engine.globalObject ());
	QScriptValue result = engine.evaluate (file.readAll(), _filename);

	if (scriptError ()) return false;

	return true;
}

void QtScriptBackendThread::run () {
	RK_TRACE (PHP);

	QScriptValue backend_object = engine.newQObject (this);
	engine.globalObject ().setProperty ("_RK_backend", backend_object);
	RKMessageCatalogObject::addI18nToScriptEngine (&engine, catalog);
	if (scriptError ()) return;

#ifdef USE_Q_SCRIPT_PROGRAM
	if (!RKPrecompiledQtScripts::loadCommonScript (&engine, _commonfile)) {
		if (!engine.hasUncaughtException ()) {
			emit error (i18n ("Could not open common script file \"%1\"", _commonfile));
		} else {
			scriptError ();
		}
		return;
	}
#else
	if (!includeFile (_commonfile)) return;
#endif
	if (!includeFile (_scriptfile)) return;

	emit (commandDone ("startup complete"));

	QString command;
	while (1) {
		if (killed) return;
		if (sleeping) {
			sleep_mutex.lock ();
			sleep_mutex.unlock ();
		}

		mutex.lock ();
		if (!_command.isNull ()) {
			command = _command;
			_command.clear ();
		}
		mutex.unlock ();

		if (command.isNull ()) {
			msleep (1);
			continue;
		}

		// do it!
		QScriptValue result = engine.evaluate (command);
		if (scriptError ()) return;
		emit (commandDone (result.toString ()));

		command.clear ();
	}
}

#ifdef USE_Q_SCRIPT_PROGRAM
namespace RKPrecompiledQtScripts {
	QMap<QString, QScriptProgram> compiled_includes;
	QMutex compiled_includes_mutex;

	bool loadCommonScript (QScriptEngine* engine, QString scriptfile) {
		RK_TRACE (PHP);

		QMutexLocker ml (&compiled_includes_mutex);
		if (!compiled_includes.contains (scriptfile)) {
			RK_DEBUG (PHP, DL_DEBUG, "Compiling common script file %s", qPrintable (scriptfile));
			QFile file (scriptfile);
			if (!file.open (QIODevice::ReadOnly | QIODevice::Text)) {
				return false;
			}
			compiled_includes.insert (scriptfile, QScriptProgram (file.readAll (), scriptfile));
			file.close ();
		} else {
			RK_DEBUG (PHP, DL_DEBUG, "Script file %s is already compiled", qPrintable (scriptfile));
		}
		engine->evaluate (compiled_includes[scriptfile]);
		if (engine->hasUncaughtException ()) {
			compiled_includes.remove (scriptfile);
			return false;
		}
		return true;
	}
}
#endif

#include "qtscriptbackend.moc"
