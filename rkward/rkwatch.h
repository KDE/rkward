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

#include "misc/rktogglewidget.h"

#include <qstring.h>

class RInterface;
class RCommand;
class RCommand;
class QPushButton;
class QTextEdit;
class RKCommandEditor;
class QBoxLayout;

/**
	\brief This widget shows all executed commands and their result
@author Thomas Friedrichsmeier
*/

class RKwatch : public RKToggleWidget {
	Q_OBJECT
public: 
	RKwatch(RInterface *parent);
	~RKwatch();
/** Adds input to the watch-window (i.e. commands issued) */
	void addInput (RCommand *command);
/** Adds output to the watch-window (i.e. replies received) */
	void addOutput (RCommand *command);
public slots:
/** configures the watch-window */
	void configureWatch ();
/** clears the watch-window */
	void clearWatch ();

private:
	void addInputNoCheck (RCommand *command);
/** Pointer to the R-Interface */
	RInterface *r_inter;

	QTextEdit *watch;
	QBoxLayout* pLayout;
};

#endif
