/***************************************************************************
                          rkward_startup_wrapper  -  description
                             -------------------
    begin                : Sun Mar 10 2013
    copyright            : (C) 2013 by Thomas Friedrichsmeier
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

#ifndef RKWARD_REL_INSTALL_PATH
#	define RKWARD_REL_INSTALL_PATH ""
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

int main (int argc, char *argv[]) {
	QApplication app (argc, argv);
	QStringList args = app.arguments ();
	if (!args.isEmpty ()) args.pop_front ();	// The command itself

	QString kdeinit4_exe;
	QString rkward_frontend_exe;
	rkward_frontend_exe = findRKWardAtPath (app.applicationDirPath ());	// these two are for running directly from a build tree
	if (rkward_frontend_exe.isNull ()) rkward_frontend_exe = findRKWardAtPath (QDir (app.applicationDirPath ()).filePath (".."));	// these two are for running directly from a build tree
	if (rkward_frontend_exe.isNull ()) {	// this is for the regular case: startup wrapper is not in the same dir as rkward.frontend
		QString kde4_config_exe;
		kde4_config_exe = findExeAtPath ("kde4-config", QDir::currentPath ());
		if (kde4_config_exe.isNull ()) kde4_config_exe = findExeAtPath ("kde4-config", app.applicationDirPath ());
		if (kde4_config_exe.isNull ()) kde4_config_exe = findExeAtPath ("kde4-config", QDir (app.applicationDirPath ()).filePath ("KDE/bin"));

		if (kde4_config_exe.isNull ()) {
			QMessageBox::critical (0, "Could not find KDE installation", "The KDE installation could not be found (kde4-config). When moving / copying RKWard, make sure to copy the whole application folder, or create a shorcut / link, instead.");
			exit (1);
		}

		QDir kde_dir (QFileInfo (kde4_config_exe).absolutePath ());
		kde_dir.makeAbsolute ();
#ifdef Q_WS_WIN
		kdeinit4_exe = findExeAtPath ("kdeinit4", kde_dir.path ());
		qputenv ("PATH", QString (kde_dir.path () + ";" + qgetenv ("PATH")).toLocal8Bit ());
#endif
		// important if RKWard is not in KDEPREFIX/bin but e.g. KDEPREFIX/lib/libexec
		qputenv ("RKWARD_ENSURE_PREFIX", kde_dir.path().toLocal8Bit ());
	
		rkward_frontend_exe = findRKWardAtPath (kde_dir.absoluteFilePath ("bin"));
		if (rkward_frontend_exe.isNull ()) rkward_frontend_exe = findRKWardAtPath (kde_dir.absoluteFilePath ("../lib/libexec"));
		if (rkward_frontend_exe.isNull ()) rkward_frontend_exe = findRKWardAtPath (kde_dir.absoluteFilePath (RKWARD_REL_INSTALL_PATH));

		if (rkward_frontend_exe.isNull ()) {
			QMessageBox::critical (0, "RKWard frontend binary missing", "RKWard frontend binary could not be found. When moving / copying RKWard, make sure to copy the whole application folder, or create a shorcut / link, instead.");
			exit (1);
		}
	}

#ifdef Q_WS_WIN
	// Explicit initialization of KDE, in case Windows 7 asks for admin priviledges
	if (kdeinit4_exe.isNull ()) {
		kdeinit4_exe = findExeAtPath ("kdeinit4", QFileInfo (rkward_frontend_exe).absolutePath ());
	}
	if (!kdeinit4_exe.isNull ()) QProcess::execute (kdeinit4_exe, QStringList ());
#endif

	bool usage = false;
	QString debugger_arg;
	QString r_exe_arg;

	for (int i=0; i < args.size (); ++i) {
		if (args[i] == "--debugger") {
			if ((i+1) < args.size ()) {
				debugger_arg = args.takeAt (i + 1);
			} else usage = true;
			args.removeAt (i);
			--i;
		} else if (args[i] == "--R") {
			if ((i+1) < args.size ()) {
				r_exe_arg = args.takeAt (i + 1);
			} else usage = true;
			args.removeAt (i);
			--i;
		}
	}

	if (usage) {
		QProcess::execute (rkward_frontend_exe, QStringList ("--help"));
		exit (1);
	}

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
		}
		if (r_exe.isNull ()) {
			r_exe = R_EXECUTABLE;
			if (!QFileInfo (r_exe).isExecutable ()) {
				QMessageBox::critical (0, "Specified R executable does not exist", QString ("The R executable specified at compile time (%1) does not exist or is not executable. Probably the installation of R has moved. You can use the command line parameter '--R', or supply an rkward.ini file to specify the new location.").arg (r_exe));
				exit (1);
			}
		}
	}

	qputenv ("R_BINARY", r_exe.toLocal8Bit ());
	QStringList call_args ("CMD");
	if (!debugger_arg.isNull ()) call_args.append (debugger_arg);
	call_args.append (rkward_frontend_exe);
	call_args.append (args);
	
	return (QProcess::execute (r_exe, call_args));
}
