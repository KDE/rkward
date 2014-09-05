/***************************************************************************
                          rkward_startup_wrapper  -  description
                             -------------------
    begin                : Sun Mar 10 2013
    copyright            : (C) 2013, 2014 by Thomas Friedrichsmeier
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

/** This simple helper executable is responsible for reading basic configuration
 * settings, checking for some services and starting RKWard. These things used
 * to be done using a script, and there was nothing wrong with that, in principle.
 * However, the binary allows more flexibility, and more consistency across
 * platforms. */

#include <QApplication>
#include <QMessageBox>
#include <QDir>
#include <QProcess>
#include <QSettings>
#include <QUrl>
#include <QFile>

#ifndef RKWARD_FRONTEND_LOCATION
#	define RKWARD_FRONTEND_LOCATION ""
#endif

#ifndef R_EXECUTABLE
#	define R_EXECUTABLE ""
#endif

QString findExeAtPath (const QString appname, const QString &path) {
	QDir dir (path);
	dir.makeAbsolute ();
	if (QFileInfo (dir.filePath (appname)).isExecutable ()) return dir.filePath (appname);
	if (QFileInfo (dir.filePath (appname + ".exe")).isExecutable ()) return dir.filePath (appname + ".exe");
	if (QFileInfo (dir.filePath (appname + ".com")).isExecutable ()) return dir.filePath (appname + ".com");
	if (QFileInfo (dir.filePath (appname + ".bat")).isExecutable ()) return dir.filePath (appname + ".bat");
	return QString ();
}

QString findRKWardAtPath (const QString &path) {
	return findExeAtPath ("rkward.frontend", path);
}

QString quoteCommand (const QString &orig) {
#ifdef Q_WS_WIN
	QString ret = "\"";
	for (int i = 0; i < orig.size (); ++i) {
		if (orig[i] == QLatin1Char ('"')) ret.append ("\"\"");
		else if (orig[i] == QLatin1Char ('\\')) ret.append ("\\\\");
		else ret.append (orig[i]);
	}
	ret.append (QLatin1Char ('"'));
	return ret;
#else
	return orig;
#endif
}

#ifndef Q_WS_WIN
// see http://blog.qt.digia.com/blog/2006/03/16/starting-interactive-processes-with-qprocess/
// Need an interactive process e.g. for running through gdb
#	include <unistd.h>
class InteractiveProcess : public QProcess {
    static int stdinClone;
public:
    InteractiveProcess (QObject *parent = 0) : QProcess (parent) {
        if (stdinClone == -1) stdinClone = ::dup (fileno(stdin));
    }
protected:
    void setupChildProcess () {
        ::dup2 (stdinClone, fileno(stdin));
    }
};
int InteractiveProcess::stdinClone = -1;
#else
// no easy solution for Windows. But ain't Windows the world of graphical debuggers, anyway...
#	define InteractiveProcess QProcess
#endif

int main (int argc, char *argv[]) {
	QApplication app (argc, argv);
	QStringList args = app.arguments ();
	if (!args.isEmpty ()) args.pop_front ();	// The command itself

	// Parse arguments that need handling in the wrapper
	bool usage = false;
	QString debugger_arg;
	QString r_exe_arg;
	int debug_level = 2;

	for (int i=0; i < args.size (); ++i) {
		if (args[i] == "--debugger") {
			if ((i+1) < args.size ()) {
				debugger_arg = args.takeAt (i + 1);
			} else usage = true;
			args.removeAt (i);
			--i;
		} else if (args[i] == "--r-executable") {
			if ((i+1) < args.size ()) {
				r_exe_arg = args.takeAt (i + 1);
			} else usage = true;
			args.removeAt (i);
			--i;
		} else if (args[i] == "--debug-level") {
			if ((i+1) < args.size ()) {
				debug_level = args[i+1].toInt ();
			}
		}
	}

	// MacOS may need some path adjustments, first
#ifdef Q_WS_MAC
	QString oldpath = qgetenv ("PATH");
	if (!oldpath.contains (INSTALL_PATH)) {
		//ensure that PATH is set to include what we deliver with the bundle
		qputenv ("PATH", QString ("\"%1/bin\":\"%1/sbin\":%2").arg (INSTALL_PATH).arg (oldpath).toLocal8Bit ());
	}
	// ensure that RKWard finds its own packages
	qputenv ("R_LIBS"=R_LIBS);
	QProcess::execute ("lanuchctl", QStringList () << "load" << "-w" << "\"" INSTALL_PATH "/Library/LaunchAgents/org.freedesktop.dbus-session.plist\"");
#endif

	// Locate KDE and RKWard installations
	QString kdeinit4_exe;
	QString rkward_frontend_exe;
	rkward_frontend_exe = findRKWardAtPath (app.applicationDirPath ());	// this is for running directly from a build tree
	if (rkward_frontend_exe.isNull ()) {	// this is for the regular case: startup wrapper is not in the same dir as rkward.frontend
		QString kde4_config_exe;
		kde4_config_exe = findExeAtPath ("kde4-config", QDir::currentPath ());
		if (kde4_config_exe.isNull ()) kde4_config_exe = findExeAtPath ("kde4-config", app.applicationDirPath ());
		if (kde4_config_exe.isNull ()) kde4_config_exe = findExeAtPath ("kde4-config", QDir (app.applicationDirPath ()).filePath ("KDE/bin"));
		if (kde4_config_exe.isNull ()) {
#ifdef Q_WS_WIN
			QStringList syspath = QString (qgetenv ("PATH")).split (";");
#else
			QStringList syspath = QString (qgetenv ("PATH")).split (":");
#endif
			for (int i = 0; i < syspath.size (); ++i) {
				kde4_config_exe = findExeAtPath ("kde4-config", syspath[i]);
				if (!kde4_config_exe.isNull ()) break;
			}
		}

		if (kde4_config_exe.isNull ()) {
			QMessageBox::critical (0, "Could not find KDE installation", "The KDE installation could not be found (kde4-config). When moving / copying RKWard, make sure to copy the whole application folder, or create a shorcut / link, instead.");
			exit (1);
		}

		QDir kde_dir (QFileInfo (kde4_config_exe).absolutePath ());
		kde_dir.makeAbsolute ();
#ifdef Q_WS_WIN
		kdeinit4_exe = findExeAtPath ("kdeinit4", kde_dir.path ());
		qputenv ("PATH", QString (kde_dir.path () + ";" + qgetenv ("PATH")).toLocal8Bit ());
		if (debug_level > 3) qDebug ("Adding %s to the system path", qPrintable (kde_dir.path ()));
#endif
		// important if RKWard is not in KDEPREFIX/bin but e.g. KDEPREFIX/lib/libexec
		qputenv ("RKWARD_ENSURE_PREFIX", kde_dir.path().toLocal8Bit ());
		if (debug_level > 3) qDebug ("Setting environment variable RKWARD_ENSURE_PREFIX=%s", qPrintable (kde_dir.path ()));
qDebug ("%s", RKWARD_FRONTEND_LOCATION);
		rkward_frontend_exe = findRKWardAtPath (RKWARD_FRONTEND_LOCATION);
		if (rkward_frontend_exe.isNull ()) rkward_frontend_exe = findRKWardAtPath (kde_dir.absoluteFilePath ("bin"));
		if (rkward_frontend_exe.isNull ()) rkward_frontend_exe = findRKWardAtPath (kde_dir.absoluteFilePath ("../lib/libexec"));

		if (rkward_frontend_exe.isNull ()) {
			QMessageBox::critical (0, "RKWard frontend binary missing", "RKWard frontend binary could not be found. When moving / copying RKWard, make sure to copy the whole application folder, or create a shorcut / link, instead.");
			exit (1);
		}
	}

	if (usage) {
		QProcess::execute (rkward_frontend_exe, QStringList ("--help"));
		exit (1);
	}

#ifdef Q_WS_WIN
	// Explicit initialization of KDE, in case Windows 7 asks for admin priviledges
	if (kdeinit4_exe.isNull ()) {
		kdeinit4_exe = findExeAtPath ("kdeinit4", QFileInfo (rkward_frontend_exe).absolutePath ());
	}
	if (!kdeinit4_exe.isNull ()) QProcess::execute (kdeinit4_exe, QStringList ());
#else
// Apparently on some systems an embedded R gets outsmarted somehow, and LC_NUMERIC is set to some dangerous value for the whole app (via SCIM)
// To prevent this, set it here, explicitely. R does not work with wrong settings of LC_NUMERIC.

// First, however, need to unset LC_ALL, if set. Instead we set LANG, so the default will be the same, where not overridden
	QString lcall = qgetenv ("LC_ALL");
	if (!lcall.isEmpty ()) {
		qputenv ("LANG", lcall.toLocal8Bit ());
		qputenv ("LC_ALL", "");
		qDebug ("Warning: Unsetting LC_ALL");
	}

	qputenv ("LC_NUMERIC", "C");
#endif

	// Look for R:
	//- command line parameter
	//- Specified in cfg file next to rkward executable
	//- compile-time default
	QString r_exe = r_exe_arg;
	if (!r_exe.isNull ()) {
		if (!QFileInfo (r_exe).isExecutable ()) {
			QMessageBox::critical (0, "Specified R executable does not exist", QString ("The R executable specified on the command line (%1) does not exist or is not executable.").arg (r_exe));
			exit (1);
		}
		if (debug_level > 3) qDebug ("Using R specified on command line");
	} else {
		QFileInfo frontend_info (rkward_frontend_exe);
		QDir frontend_path = frontend_info.absoluteDir ();
		QFileInfo rkward_ini_file (frontend_path.absoluteFilePath ("rkward.ini"));
		if (rkward_ini_file.isReadable ()) {
			QSettings rkward_ini (rkward_ini_file.absoluteFilePath (), QSettings::IniFormat);
			r_exe = rkward_ini.value ("R executable").toString ();
			if (!r_exe.isNull ()) {
				if (QDir::isRelativePath (r_exe)) {
					r_exe = frontend_path.absoluteFilePath (r_exe);
				}
				if (!QFileInfo (r_exe).isExecutable ()) {
					QMessageBox::critical (0, "Specified R executable does not exist", QString ("The R executable specified in the rkward.ini file (%1) does not exist or is not executable.").arg (rkward_ini_file.absoluteFilePath ()));
					exit (1);
				}
			}
			if (debug_level > 3) qDebug ("Using R as configured in config file %s", qPrintable (rkward_ini_file.absoluteFilePath ()));
		}
		if (r_exe.isNull ()) {
			r_exe = R_EXECUTABLE;
			if (!QFileInfo (r_exe).isExecutable ()) {
				QMessageBox::critical (0, "Specified R executable does not exist", QString ("The R executable specified at compile time (%1) does not exist or is not executable. Probably the installation of R has moved. You can use the command line parameter '--R', or supply an rkward.ini file to specify the new location.").arg (r_exe));
				exit (1);
			}
			if (debug_level > 3) qDebug ("Using R as configured at compile time");
		}
	}

	qputenv ("R_BINARY", r_exe.toLocal8Bit ());
	QStringList call_args ("CMD");
	if (!debugger_arg.isNull ()) call_args.append (debugger_arg.split (" "));
	call_args.append (quoteCommand (rkward_frontend_exe));

	if (!args.isEmpty ()) {
		// NOTE: QProcess quotes its arguments, *but* properly passing all spaces and quotes through the R CMD wrapper, seems near(?) impossible on Windows. Instead, we use percent encoding, internally.
		for (int i = 0; i < args.size (); ++i) {
			call_args.append (QString::fromUtf8 (QUrl::toPercentEncoding (args[i], QByteArray (), " \"")));
		}
	}

	if (debug_level > 2) qDebug ("Starting frontend: %s %s", qPrintable (r_exe), qPrintable (call_args.join (" ")));

	InteractiveProcess proc;
	proc.setProcessChannelMode (QProcess::ForwardedChannels);
	proc.start (quoteCommand (r_exe), call_args);
	bool ok = proc.waitForFinished (-1);
	if (proc.exitCode () || !ok) {
		QMessageBox::critical (0, "Error starting RKWard", QString ("Starting RKWard failed with error \"%1\"").arg (proc.errorString ()));
	}

	return (0);
}
