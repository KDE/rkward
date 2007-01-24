/***************************************************************************
                          scriptbackend  -  description
                             -------------------
    begin                : Sun Aug 15 2004
    copyright            : (C) 2004, 2007 by Thomas Friedrichsmeier
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
#ifndef SCRIPTBACKEND_H
#define SCRIPTBACKEND_H

#include <qobject.h>

#include <qstring.h>

class RKComponentPropertyCode;

/**
Abstract base class for scripting-language backends. Mostly pure virtual functions only.

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
		Cleanup = 3,
		Preview = 4,
		Ignore = 5,
		User = 6
	};

/** initialize backend
@param filename Filename of the template to work on
@param code_property If you supply a pointer to an RKComponentPropertyCode, The backend will directly set values for this property in response to calls to preproces (), calculate (), printout (), and cleanup ().
@param add_headings (Only meaningful, if code_property is not 0). If set to true, heading comments will be added to each section of the code (e.g. "## Do calculations")
@returns true on successful initialization, false on errors */
	virtual bool initialize (const QString &filename, RKComponentPropertyCode *code_property=0, bool add_headings=true) = 0;
	virtual void destroy () = 0;
	
	virtual void preprocess (int flags) = 0;
	virtual void calculate (int flags) = 0;
	virtual void printout (int flags) = 0;
	virtual void cleanup (int flags) = 0;
	virtual void preview (int flags) = 0;
	
	virtual bool isBusy () { return busy; };
	
	virtual void writeData (const QString &data) = 0;
	
	QString retrieveOutput () { return _output; };
	void resetOutput () { _output = QString::null; };
signals:
	void commandDone (int);
	void idle ();
	void requestValue (const QString &);
	void haveError ();
protected:
	RKComponentPropertyCode *code_property;
	bool add_headings;
	QString _output;
	bool busy;
};

#endif
