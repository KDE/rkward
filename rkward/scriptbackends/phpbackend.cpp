/***************************************************************************
                          phpbackend  -  description
                             -------------------
    begin                : Mon Jul 26 2004
    copyright            : (C) 2004, 2007, 2009 by Thomas Friedrichsmeier
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
#include "phpbackend.h"

#include "stdio.h"

#include <qfile.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <QTimer>

#include <kmessagebox.h>
#include <klocale.h>

#include "../misc/rkcommonfunctions.h"
#include "../settings/rksettingsmodulephp.h"
#include "../plugin/rkcomponentproperties.h"
#include "../debug.h"

PHPBackend::PHPBackend (const QString &filename) : ScriptBackend () {
	RK_TRACE (PHP);

	php_process = 0;
	dead = false;
	eot_string="#RKEND#\n";
	eoq_string="#RKQEND#\n";

	PHPBackend::filename = filename;
}


PHPBackend::~PHPBackend () {
	RK_TRACE (PHP);

	if (php_process) {
		RK_DO (qDebug ("PHP process has not exited after being killed for ten seconds. We destroy it now, hoping for the best"), PHP, DL_ERROR);
	}
}

bool PHPBackend::initialize (RKComponentPropertyCode *code_property, bool add_headings) {
	RK_TRACE (PHP);

	if (php_process) {
		RK_DO (qDebug ("another template is already openend in this backend"), PHP, DL_ERROR);
		return false;
	}

	QDir files_path (RKCommonFunctions::getRKWardDataDir () + "phpfiles/");
	QString common_php = files_path.absoluteFilePath ("common.php");
	QString php_ini = files_path.absoluteFilePath ("php.ini");	// make sure to set good options
	if (!QFileInfo (common_php).isReadable ()) {
		KMessageBox::error (0, i18n ("The support file \"%1\" could not be found or is not readable. Please check your installation.", common_php), i18n ("PHP-Error"));
		emit (haveError ());
		return false;
	}

	php_process = new QProcess (this);
	connect (php_process, SIGNAL (readyRead()), this, SLOT (gotOutput()));
	connect (php_process, SIGNAL (error(QProcess::ProcessError)), this, SLOT (processError(QProcess::ProcessError)));
	connect (php_process, SIGNAL (finished(int,QProcess::ExitStatus)), this, SLOT (processDead(int,QProcess::ExitStatus)));

	php_process->start ("\"" + RKSettingsModulePHP::phpBin() + "\"" + " -c \"" + php_ini + "\" \"" + common_php + "\"");

	doing_command = startup_done = false;
	busy = true;

	// start the real template
	callFunction ("include (\"" + filename + "\");", 0, Ignore);

	PHPBackend::code_property = code_property;
	PHPBackend::add_headings = add_headings;
	return true;
}

void PHPBackend::destroy () {
	RK_TRACE (PHP);
	if (!dead) {
		dead = true;
		if (php_process) php_process->kill ();
		QTimer::singleShot (10000, this, SLOT (deleteLater()));	// don't wait for ever for the process to die, even if it's somewhat dangerous
		code_property = 0;
	}

	busy = false;
}

void PHPBackend::tryNextFunction () {
	RK_TRACE (PHP);

	if (php_process && (!dead) && (!busy) && (!command_stack.isEmpty ())) {
	/// clean up previous command if applicable
		if (command_stack.first ()->complete) {
			delete command_stack.takeFirst ();
			
			if (!command_stack.count ()) return;
		}
		
		RK_DO (qDebug ("submitting PHP code: %s", command_stack.first ()->command.toLatin1 ().data ()), PHP, DL_DEBUG);
		php_process->write (QString (command_stack.first ()->command + eot_string).toLatin1 ());
		doing_command = busy = true;
		command_stack.first ()->complete = true;
		current_flags = command_stack.first ()->flags;
		current_type = command_stack.first ()->type;
	}
}

void PHPBackend::writeData (const QString &data) {
	RK_TRACE (PHP);

	RK_DO (qDebug ("submitting data: %s", data.toLatin1 ().data ()), PHP, DL_DEBUG);
	php_process->write (QString (data + eot_string).toLatin1 ());
	tryNextFunction ();
}

void PHPBackend::gotOutput () {
	RK_TRACE (PHP);

	qint64 len = php_process->bytesAvailable ();
	if (!len) return;	// false alarm

	QByteArray buf = php_process->readAll ();
	RK_DO (qDebug ("PHP transmission:\n%s", buf.data ()), PHP, DL_DEBUG);

	output_raw_buffer.append (QString::fromLatin1 (buf));
	QString request;
	QString data;
	int i, j;
	bool have_data = true;
	bool have_request = false;

	// is there a request in the output stream?
	if ((i = output_raw_buffer.indexOf (eot_string)) >= 0) {
		if ((j = output_raw_buffer.indexOf (eoq_string, i)) >= 0) {
			have_request = true;
			// is there also pending data?
			if (i) {
				data = output_raw_buffer.left (i);
			} else {
				have_data = false;
			}
			int start = i + eot_string.length ();
			request = output_raw_buffer.mid (start, j - start);
			output_raw_buffer = QString::null;
		}
	} else {
		data = output_raw_buffer;
	}
	
	// pending data is always first in a stream, so process it first, too
	if (have_data) {
		if (!startup_done) {
			php_process->close ();
			KMessageBox::error (0, i18n ("There has been an error\n(\"%1\")\nwhile starting up the PHP backend. Most likely this is due to either a bug in RKWard or a problem with your PHP installation. Check the settings (Settings->Configure Settings->PHP backend) and try again.", output_raw_buffer.trimmed ()), i18n ("PHP-Error"));
			emit (haveError ());
			destroy ();
			return;
		}
	}

	if (!have_request) {
		// output is not finished, yet.
		// return and wait for more data to come in
		RK_DO (qDebug ("PHP transmission not complete, yet"), PHP, DL_DEBUG);
		return;
	} else {
		_output.append (data);
		RK_DO (qDebug ("request: %s\ndata: %s", request.toLatin1 ().data (), data.toLatin1 ().data ()), PHP, DL_DEBUG);

		if (request == "requesting code") {
			startup_done = true;
			RK_DO (qDebug ("got type: %d, stack %d", current_type, command_stack.count ()), PHP, DL_DEBUG);
#ifdef Q_OS_WIN
			commandFinished (_output.replace ("\r\n", "\n"));
#else
			commandFinished (_output);
#endif
			_output = QString::null;
		} else if (request.startsWith ("requesting data:")) {
			QString requested_object = request.remove ("requesting data:");
			RK_DO (qDebug ("requested data: \"%s\"", requested_object.toLatin1 ().data ()), PHP, DL_DEBUG);
			emit (requestValue (requested_object));
			busy = true;
//			writeData (res + eot_string);
		} else if (request.startsWith ("PHP-Error")) {
			QString error = request.remove ("PHP-Error");
			php_process->close ();
			KMessageBox::error (0, i18n ("The PHP-backend has reported an error\n(\"%1\")\nand has been shut down. This is most likely due to a bug in the plugin. But of course you may want to try to close and restart the plugin to see whether it works with different settings.", error.trimmed ()), i18n ("PHP-Error"));
			emit (haveError ());
			destroy ();
			return;
		} else {
			RK_DO (qDebug ("unrecognized request from PHP backend: \"%s\"", request.toLatin1().data ()), PHP, DL_ERROR);
		}
	}
}

void PHPBackend::processError (QProcess::ProcessError error) {
	RK_TRACE (PHP);

	if (dead) return;	// we are already dead, so we've shown an error before.

	php_process->close ();

	if (error == QProcess::FailedToStart) {
		KMessageBox::error (0, i18n ("The PHP backend could not be started. Check whether you have correctly configured the location of the PHP-binary (Settings->Configure Settings->PHP backend)"), i18n ("PHP-Error"));
	} else {
		KMessageBox::error (0, i18n ("The PHP-backend has died unexpectedly. The current output buffer is shown below:\n%1", output_raw_buffer), i18n ("PHP Process exited"));
	}

	emit (haveError ());
	destroy ();
}

void PHPBackend::processDead (int, QProcess::ExitStatus) {
	RK_TRACE (PHP);

	if (dead) {
		php_process = 0;
		deleteLater ();
	} else destroy ();
}

#include "phpbackend.moc"
