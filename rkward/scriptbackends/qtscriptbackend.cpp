/*
qtscriptbackend - This file is part of RKWard (https://rkward.kde.org). Created: Mon Sep 28 2009
SPDX-FileCopyrightText: 2009-2015 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "qtscriptbackend.h"

#include <QTimer>
#include <QDir>

#include "KLocalizedString"
#include "kmessagebox.h"

#include "../misc/rkcommonfunctions.h"
#include "qtscripti18n.h"

#include "../debug.h"

QtScriptBackend::QtScriptBackend (const QString &filename, const RKMessageCatalog *catalog) : ScriptBackend () {
	RK_TRACE (PHP);

	script_thread = nullptr;
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

#ifdef JSBACKEND_PERFORMANCE_TEST
#include <QElapsedTimer>
#include <QApplication>
#include <QObjectCleanupHandler>
#include "../misc/rkmessagecatalog.h"
void QtScriptBackend::_performanceTest() {
	RK_DEBUG(PHP, DL_WARNING, "Starting QtScriptBackend performance test");
	QElapsedTimer t;
	QObjectCleanupHandler h;
	t.start();
	for (int i = 0; i < 1000; ++i) {
		auto b = new QtScriptBackend(QString(), RKMessageCatalog::nullCatalog());
		b->dead = true; // so it doesn't try to show an error message
		h.add(b);
		b->initialize(nullptr, false);
		connect(b->script_thread, &QtScriptBackendThread::error, b, &QObject::deleteLater);  // thread will throw an error, when trying to include non-existent script file
	}
	while (!h.isEmpty()) {
		qApp->processEvents();
	}
	RK_DEBUG(PHP, DL_WARNING, "Time to init 100 backends: %d", t.elapsed());
	RK_DEBUG(PHP, DL_WARNING, "QtScriptBackend performance test end");
}
#endif

bool QtScriptBackend::initialize (RKComponentPropertyCode *code_property, bool add_headings) {
	RK_TRACE (PHP);

	if (script_thread) {
		RK_DEBUG (PHP, DL_ERROR, "another template is already opened in this backend");
		return false;
	}

	QDir files_path (RKCommonFunctions::getRKWardDataDir () + "phpfiles/");
	QString common_js (files_path.absoluteFilePath ("common.js"));

	script_thread = new QtScriptBackendThread (common_js, filename, this, catalog);
	connect (script_thread, &QtScriptBackendThread::error, this, &QtScriptBackend::threadError);
	connect (script_thread, &QtScriptBackendThread::commandDone, this, &QtScriptBackend::commandDone);
	connect (script_thread, &QtScriptBackendThread::needData, this, &QtScriptBackend::needData);
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
		code_property = nullptr;
		if (script_thread) script_thread->kill ();
		QTimer::singleShot(10000, this, [this](){ deleteLater(); });	// don't wait for ever for the process to die, even if it's somewhat dangerous
	}

	busy = false;
}

void QtScriptBackend::tryNextFunction () {
	RK_TRACE (PHP);

	if (script_thread && (!busy) && (!command_stack.empty())) {
	/// clean up previous command if applicable
		if (command_stack.front()->complete) {
			delete command_stack.front();
			command_stack.pop_front();
			
			if (command_stack.empty ()) {
				script_thread->goToSleep (true);
				return;
			}
		}
		
		RK_DEBUG(PHP, DL_DEBUG, "submitting QtScript code: %s", qPrintable(command_stack.front()->command));
		if (script_thread) script_thread->goToSleep (false);
		script_thread->setCommand(command_stack.front()->command);
		busy = true;
		command_stack.front()->complete = true;
		current_flags = command_stack.front()->flags;
		current_type = command_stack.front()->type;
	} else {
		if (script_thread && command_stack.empty()) script_thread->goToSleep (true);
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

	KMessageBox::error (nullptr, i18n ("The QtScript-backend has reported an error:\n%1", message), i18n ("Scripting error"));

	Q_EMIT haveError();
	destroy ();
}

void QtScriptBackend::commandDone (const QString &result) {
	RK_TRACE (PHP);

	commandFinished (result);
}

void QtScriptBackend::needData (const QString &identifier, const int hint) {
	RK_TRACE (PHP);

	Q_EMIT requestValue(identifier, hint);
}


/////////////////////////////////////////////////////////////////////////////////////////

#include <QUrl>

QtScriptBackendThread::QtScriptBackendThread (const QString &commonfile, const QString &scriptfile, QtScriptBackend *parent, const RKMessageCatalog *catalog) : QThread (parent), engine(nullptr) {
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

	Q_EMIT needData(identifier, hint);
	QVariant ret;
	while (true) {
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

QVariant QtScriptBackendThread::getUiLabelPair (const QString &identifier) {
	return getValue (identifier, RKStandardComponent::UiLabelPair);
}

bool QtScriptBackendThread::scriptError(const QJSValue &val) {
	RK_TRACE (PHP);

	if (!val.isError()) return false;

	QString message = i18n("Script Error in %1, line %2: %3\nBacktrace:\n%4", val.property("fileName").toString(), val.property("lineNumber").toInt(), val.toString(), val.property("stack").toString());  // TODO: correct?
	Q_EMIT error(message);

	return true;
}

bool QtScriptBackendThread::includeFile (const QString &filename) {
	RK_TRACE (PHP);

	QString _filename = filename;
	if (QFileInfo (filename).isRelative ()) {
		QUrl script_path = QUrl (QUrl::fromLocalFile (_scriptfile)).adjusted (QUrl::RemoveFilename).resolved (QUrl (filename));
		_filename = script_path.toLocalFile ();
	}

	QFile file(_filename);
	if (!file.open (QIODevice::ReadOnly | QIODevice::Text)) {
		Q_EMIT error(i18n("The file \"%1\" (needed by \"%2\") could not be found. Please check your installation.", _filename, _scriptfile));
		return false;
	}

	QJSValue result = engine.evaluate (QString::fromUtf8 (file.readAll ()), _filename);
	if (scriptError(result)) return false;

	return true;
}

void QtScriptBackendThread::run () {
	RK_TRACE (PHP);

	QJSValue backend_object = engine.newQObject (this);
	engine.globalObject ().setProperty ("_RK_backend", backend_object);
	RKMessageCatalogObject::addI18nToScriptEngine (&engine, catalog);

	if (!includeFile (_commonfile)) return;  // TODO: import this as a module and re-use the engine in the next thread?
	if (!includeFile (_scriptfile)) return;

	Q_EMIT commandDone("startup complete");

	QString command;
	while (true) {
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
		QJSValue result = engine.evaluate (command);
		if (scriptError(result)) return;
		Q_EMIT commandDone(result.toString());

		command.clear ();
	}
}
