/*
rkdebughandler - This file is part of the RKWard project. Created: Wed Oct 19 2011
SPDX-FileCopyrightText: 2011 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKDEBUGHANDLER_H
#define RKDEBUGHANDLER_H

#include <QObject>

#include "../rbackend/rcommand.h"

class RBackendRequest;

/** A central handler, responsible for keeping all debug related widgets up-to-date */
class RKDebugHandler : public QObject {
	Q_OBJECT
public:
	explicit RKDebugHandler (QObject *parent);
	~RKDebugHandler ();

	static RKDebugHandler *instance () { return _instance; };

	void debugCall (RBackendRequest *request, RCommand *command);
	void submitDebugString (const QString &command);
	void sendCancel ();
	void endDebug ();

	enum DebugState {
		NotInDebugger,
		InDebugPrompt,
		InDebugRun
	};
	DebugState state () const { return _state; };

	QString outputContext () const { return _output_context; };
	QStringList calls () const { return _calls; };
	QStringList functions () const { return _functions; };
	QStringList environments () const { return _environments; };
	QStringList locals () const { return _locals; };
	QList<int> relativeSourceLines () const { return _rel_src_lines; };
	QString debugPrompt () const { return _prompt; };
	RCommand *command () const { return _command; };
Q_SIGNALS:
	void newDebugState ();
private:
	RCommand *_command;
	QStringList _calls, _functions, _environments, _locals;
	QList<int> _rel_src_lines;
	QString _prompt, _output_context;
	DebugState _state;
	RBackendRequest *_request;

	static RKDebugHandler *_instance;
};

#endif
