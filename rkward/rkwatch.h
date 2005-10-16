/***************************************************************************
                          rkwatch.h  -  description
                             -------------------
    begin                : Sun Nov 3 2002
    copyright            : (C) 2002 by Thomas Friedrichsmeier
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

#ifndef RKWATCH_H
#define RKWATCH_H

#include <qstring.h>
#include <kmdichildview.h>

class RCommand;
class QPushButton;
class QTextEdit;
class RKCommandEditor;
class QBoxLayout;

/**
	\brief This widget shows all executed commands and their result
@author Thomas Friedrichsmeier
*/

class RKwatch : public KMdiChildView {
	Q_OBJECT
public: 
	RKwatch ();
	~RKwatch ();
/** Adds input to the watch-window (i.e. commands issued) */
	void addInput (RCommand *command);
/** Adds output to the watch-window (i.e. replies received) */
	void addOutput (RCommand *command);
signals:
/** the watch emits this, when it should be raised (apparently this can only be done from the main frame) */
	void raiseWatch ();
public slots:
/** configures the watch-window */
	void configureWatch ();
/** clears the watch-window */
	void clearWatch ();

private:
	void addInputNoCheck (RCommand *command);

	QTextEdit *watch;
	QBoxLayout* pLayout;
};

#endif
