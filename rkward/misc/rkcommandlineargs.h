/*
rkcommandlineargs - This file is part of the RKWard project. Created: Tue May 21 2024
SPDX-FileCopyrightText: 2024 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKCOMMANDLINEARGS_H
#define RKCOMMANDLINEARGS_H

#include <QVariant>
#include <QMap>
#include <QCommandLineOption>

class QCommandLineParser;
class KAboutData;
class QCoreApplication;

class RKCommandLineArgs {
public:
	explicit RKCommandLineArgs(KAboutData *about, QCoreApplication *app);
	~RKCommandLineArgs() {};
	enum Option {
		UrlArgs,
		Evaluate,
		DebugLevel,
		DebugFlags,
		DebugOutput,
		BackendDebugger,
		RExecutable,
		Reuse,
		AutoReuse,
		NoWarnExternal,
		QuirkMode,
		Setup,

		NUM_OPTIONS
	};
	void parseArgs(	QCommandLineParser *parser);
	QVariant operator[](const Option op) const {
		return storage[op];
	}
	static QVariant get(const Option op) {
		return instance->storage[op];
	};
private:
friend class RKSetupWizard;
	void set(const Option op, const QVariant value) { storage[op] = value; };
	QList<QVariant> storage;
	static RKCommandLineArgs *instance;
};

#endif
