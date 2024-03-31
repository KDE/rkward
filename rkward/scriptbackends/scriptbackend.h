/*
scriptbackend - This file is part of the RKWard project. Created: Sun Aug 15 2004
SPDX-FileCopyrightText: 2004-2012 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef SCRIPTBACKEND_H
#define SCRIPTBACKEND_H

#include <list>

#include <QObject>
#include <QString>

#include "../plugin/rkstandardcomponent.h"

class RKComponentPropertyCode;

/**
Abstract base class for scripting-language backends. Mostly pure virtual functions only + some handling to make sure the processing is asynchronous.

@author Thomas Friedrichsmeier
*/
class ScriptBackend : public QObject {
	Q_OBJECT
public:
	ScriptBackend ();

	~ScriptBackend ();

	enum CallType {
		Preprocess = 0,
		Calculate = 1,
		Printout = 2,
		Preview = 4,
		Ignore = 5,
		User = 6
	};

/** initialize backend
@param code_property If you supply a pointer to an RKComponentPropertyCode, The backend will directly set values for this property in response to calls to preproces (), calculate (), printout (), and cleanup ().
@param add_headings (Only meaningful, if code_property is not 0). If set to true, heading comments will be added to each section of the code (e.g. "## Do calculations")
@returns true on successful initialization, false on errors */
	virtual bool initialize (RKComponentPropertyCode *code_property=nullptr, bool add_headings=true) = 0;
	virtual void destroy () = 0;
	
	virtual void preprocess (int flags) = 0;
	virtual void calculate (int flags) = 0;
	virtual void printout (int flags) = 0;
	virtual void preview (int flags) = 0;
	
	virtual bool isBusy () { return busy; };
	
	virtual void writeData (const QVariant &data) = 0;
Q_SIGNALS:
	void commandDone (int);
	void idle ();
	void requestValue (const QString &, const int);
	void haveError ();
protected:
	RKComponentPropertyCode *code_property;
	bool add_headings;
	bool busy;

	struct ScriptCommand {
	/// the command string
		QString command;
	/// flags attached to this command by the parent
		int flags;
	/// internal type (used to find out, if this is a preproces, calculate, printout, or cleanup call)
		int type;
	/// whether command has finished
		bool complete;
	};
	std::list<ScriptCommand *> command_stack;

	int current_flags;
	int current_type;

/** Invalidate all previous calls of the given type */
	void invalidateCalls (int type);
/** call a function on the current template. */
	void callFunction (const QString &function, int flags, int type);

	void commandFinished (const QString &output);
	virtual void tryNextFunction () = 0;
};

#endif
