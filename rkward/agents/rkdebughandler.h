/***************************************************************************
                          rkdebughandler  -  description
                             -------------------
    begin                : Wed Oct 19 2011
    copyright            : (C) 2011 by Thomas Friedrichsmeier
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

#ifndef RKDEBUGHANDLER_H
#define RKDEBUGHANDLER_H

#include <QObject>

#include "../rbackend/rcommand.h"

class RBackendRequest;

/** A central handler, responsible for keeping all debug related widgets up-to-date */
class RKDebugHandler : public QObject {
	Q_OBJECT
public:
	RKDebugHandler (QObject *parent);
	~RKDebugHandler ();

	static RKDebugHandler *instance () { return _instance; };

	void debugCall (RBackendRequest *request, RCommand *command);
	void submitDebugString (const QString &command);
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
	QString debugPrompt () const { return _prompt; };
signals:
	void newDebugState ();
private:
	QStringList _calls, _functions, _environments, _locals;
	QString _prompt, _output_context;
	DebugState _state;
	RBackendRequest *_request;

	static RKDebugHandler *_instance;
};

#endif
