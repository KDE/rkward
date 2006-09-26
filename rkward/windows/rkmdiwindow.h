/***************************************************************************
                          rkmdiwindow  -  description
                             -------------------
    begin                : Tue Sep 26 2006
    copyright            : (C) 2006 by Thomas Friedrichsmeier
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

#ifndef RKMDIWINDOW_H
#define RKMDIWINDOW_H

#include <qwidget.h>

#include <kparts/part.h>

class RKWorkplace;

/** Base class for rkward document mdi windows */
class RKMDIWindow : public QWidget {
	Q_OBJECT
public:
	enum Type {
		DataEditorWindow=1,
		CommandEditorWindow=2,
		OutputWindow=4,
		HelpWindow=8,
		AnyType=DataEditorWindow | CommandEditorWindow | OutputWindow | HelpWindow
	};

	enum State {
		Attached=1,
		Detached=2,
		AnyState=Attached | Detached
	};
protected:
/** constructor
@param parent parent widget
@param type Type of window */
	RKMDIWindow (QWidget *parent, Type type);
	~RKMDIWindow ();
public:
	virtual bool isModified () = 0;
	virtual QString fullCaption ();
	virtual QString shortCaption ();
	virtual KParts::Part *getPart () = 0;
	void setCaption (const QString &caption);
	virtual QWidget *getWindow ();
signals:
	void captionChanged (RKMDIWindow *);
protected:
friend class RKWorkplace;
	Type type;
private:
	State state;
};

#endif
