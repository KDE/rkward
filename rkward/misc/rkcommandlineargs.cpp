/*
rkcommandlineargs - This file is part of the RKWard project. Created: Tue May 21 2024
SPDX-FileCopyrightText: 2024-2025 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "rkcommandlineargs.h"

#include <KAboutData>
#include <KLocalizedString>
#include <QCommandLineParser>
#include <QDir>
#include <QUrl>

#include "../debug.h"

RKCommandLineArgs *RKCommandLineArgs::instance = nullptr;

RKCommandLineArgs::RKCommandLineArgs(KAboutData *about, QCoreApplication *app, bool fake_for_autotests) {
	RK_TRACE(MISC);
	RK_ASSERT(instance == nullptr);

	instance = this;
	QCommandLineParser parser;
	about->setupCommandLine(&parser);

	parser.addOption(QCommandLineOption(QStringLiteral("evaluate"), i18n("After starting (and after loading the specified workspace, if applicable), evaluate the given R code."), QStringLiteral("Rcode"), QString()));
	parser.addOption(QCommandLineOption(QStringLiteral("debug-level"), i18n("Verbosity of debug messages (0-5)"), QStringLiteral("level"), QStringLiteral("2")));
	parser.addOption(QCommandLineOption(QStringLiteral("debug-flags"), i18n("Mask for components to debug (see debug.h)"), QStringLiteral("flags"), QString::number(DEBUG_ALL)));
	parser.addOption(QCommandLineOption(QStringLiteral("debug-output"), i18n("Where to send debug message (file|terminal)"), QStringLiteral("where"), QStringLiteral("file")));
	parser.addOption(QCommandLineOption(QStringLiteral("backend-debugger"), i18n("Debugger for the backend. (Enclose any debugger arguments in single quotes ('') together with the command. Make sure to re-direct stdout!)"), QStringLiteral("command"), QString()));
	parser.addOption(QCommandLineOption(QStringLiteral("r-executable"), i18n("Use specified R installation, instead of the one configured at compile time (note: rkward R library must be installed to that installation of R)"), QStringLiteral("command"), QString()));
	parser.addOption(QCommandLineOption(QStringLiteral("reuse"), i18n("Reuse a running RKWard instance (if available). If a running instance is reused, only the file arguments will be interpreted, all other options will be ignored.")));
	parser.addOption(QCommandLineOption(QStringLiteral("autoreuse"), i18n("Behaves like --reuse, if any file arguments are also given, starts a new instance, otherwise. Intended for use in the .desktop file.")));
	parser.addOption(QCommandLineOption(QStringLiteral("nowarn-external"), i18n("When used in conjunction with rkward://runplugin/-URLs specified on the command line, suppresses the warning about application-external (untrusted) links.")));
	parser.addOption(QCommandLineOption(QStringLiteral("quirkmode"), i18n("Disable some startup validation code. Experimental option, not intended for regular use.")));
	parser.addOption(QCommandLineOption(QStringLiteral("setup"), i18n("Act as if the version of RKWard had changed (show setup wizard, and (re-)install rkward R package).")));
	parser.addPositionalArgument(QStringLiteral("files"), i18n("File or files to open, typically a workspace, or an R script file. When loading several things, you should specify the workspace, first."), QStringLiteral("[Files...]"));

	if (fake_for_autotests) {
		parser.parse(app->arguments().mid(0, 1));
	} else {
		parser.process(*app);
	}
	about->processCommandLine(&parser);

	storage.resize(NUM_OPTIONS);
	storage[Evaluate] = parser.value(QStringLiteral("evaluate"));
	storage[DebugLevel] = parser.value(QStringLiteral("debug-level")).toInt();
	storage[DebugFlags] = parser.value(QStringLiteral("debug-flags")).toInt();
	storage[DebugOutput] = parser.value(QStringLiteral("debug-output"));
	storage[BackendDebugger] = parser.value(QStringLiteral("backend-debugger"));
	storage[RExecutable] = parser.value(QStringLiteral("r-executable"));
	storage[Reuse] = parser.isSet(QStringLiteral("reuse"));
	storage[AutoReuse] = parser.isSet(QStringLiteral("autoreuse"));
	storage[NoWarnExternal] = parser.isSet(QStringLiteral("nowarn-external"));
	storage[QuirkMode] = parser.isSet(QStringLiteral("quirkmode"));
	storage[Setup] = parser.isSet(QStringLiteral("setup"));
	QStringList url_args = parser.positionalArguments();
	if (!url_args.isEmpty()) {
		for (int i = 0; i < url_args.size(); ++i) {
			url_args[i] = QUrl::fromUserInput(url_args[i], QDir::currentPath(), QUrl::AssumeLocalFile).toString();
		}
	}
	storage[UrlArgs] = url_args;
}

#include "../debug.h"
