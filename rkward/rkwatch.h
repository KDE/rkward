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

#include <rkwatch_ui.h>

#include <qstring.h>

class RInterface;

/**
  *@author Thomas Friedrichsmeier
  */

class RKwatch : public RKwatchUi  {
	Q_OBJECT
public: 
	RKwatch(RInterface *parent);
	~RKwatch();
public slots:
/** Adds input to the watch-window (i.e. commands issued) */
	void slotAddInput (QString text);
/** Adds output to the watch-window (i.e. replies received) */
	void slotAddOutput (QString text);
/** Clears commands-textedit */
	void clearCommand ();
/** Submits commands in commands-textedit */
	void submitCommand ();
/** Enables the submit button */
	void enableSubmit ();
/** Disable the submit button */
	void disableSubmit ();
private:
/** Pointer to the R-Interface */
	RInterface *r_inter;
};

#endif
