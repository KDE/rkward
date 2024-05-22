/*
rkcommandlineargs - This file is part of the RKWard project. Created: Tue May 21 2024
SPDX-FileCopyrightText: 2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkcommandlineargs.h"

#include <KLocalizedString>
#include <KAboutData>
#include <QCommandLineParser>
#include <QUrl>
#include <QDir>

#include "../debug.h"

RKCommandLineArgs* RKCommandLineArgs::instance = nullptr;

RKCommandLineArgs::RKCommandLineArgs(KAboutData *about, QCoreApplication *app) {
	RK_TRACE(MISC);
	RK_ASSERT(instance == nullptr);

	instance = this;
	QCommandLineParser parser;
	about->setupCommandLine(&parser);

	parser.addOption(QCommandLineOption("evaluate", i18n("After starting (and after loading the specified workspace, if applicable), evaluate the given R code."), "Rcode", QString()));
	parser.addOption(QCommandLineOption("debug-level", i18n("Verbosity of debug messages (0-5)"), "level", "2"));
	parser.addOption(QCommandLineOption("debug-flags", i18n("Mask for components to debug (see debug.h)"), "flags", QString::number(DEBUG_ALL)));
	parser.addOption(QCommandLineOption("debug-output", i18n("Where to send debug message (file|terminal)"), "where", "file"));
	parser.addOption(QCommandLineOption("backend-debugger", i18n("Debugger for the backend. (Enclose any debugger arguments in single quotes ('') together with the command. Make sure to re-direct stdout!)"), "command", QString()));
	parser.addOption(QCommandLineOption("r-executable", i18n("Use specified R installation, instead of the one configured at compile time (note: rkward R library must be installed to that installation of R)"), "command", QString()));
	parser.addOption(QCommandLineOption("reuse", i18n("Reuse a running RKWard instance (if available). If a running instance is reused, only the file arguments will be interpreted, all other options will be ignored.")));
	parser.addOption(QCommandLineOption("autoreuse", i18n("Behaves like --reuse, if any file arguments are also given, starts a new instance, otherwise. Intended for use in the .desktop file.")));
	parser.addOption(QCommandLineOption("nowarn-external", i18n("When used in conjunction with rkward://runplugin/-URLs specified on the command line, suppresses the warning about application-external (untrusted) links.")));
	parser.addOption(QCommandLineOption("quirkmode", i18n("Disable some startup validation code. Experimental option, not intended for regular use.")));
	parser.addOption(QCommandLineOption("setup", i18n("Act as if the version of RKWard had changed (show setup wizard, and (re-)install rkward R package).")));
	parser.addPositionalArgument("files", i18n("File or files to open, typically a workspace, or an R script file. When loading several things, you should specify the workspace, first."), "[Files...]");

	parser.process(*app);
	about->processCommandLine(&parser);

	storage.resize(NUM_OPTIONS);
	storage[Evaluate] = parser.value("evaluate");
	storage[DebugLevel] = parser.value("debug-level").toInt();
	storage[DebugFlags] = parser.value("debug-flags").toInt();
	storage[DebugOutput] = parser.value("debug-output");
	storage[BackendDebugger] = parser.value("backend-debugger");
	storage[RExecutable] = parser.value("r-executable");
	storage[Reuse] = parser.isSet("reuse");
	storage[AutoReuse] = parser.isSet("autoreuse");
	storage[NoWarnExternal] = parser.isSet("nowarn-external");
	storage[QuirkMode] = parser.isSet("quirkmode");
	storage[Setup] = parser.isSet("setup");
	QStringList url_args = parser.positionalArguments ();
	if (!url_args.isEmpty ()) {
		for (int i = 0; i < url_args.size(); ++i) {
			url_args[i] = QUrl::fromUserInput(url_args[i], QDir::currentPath(), QUrl::AssumeLocalFile).toString();
		}
	}
	storage[UrlArgs] = url_args;
}

#include "../debug.h"
