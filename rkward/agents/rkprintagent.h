/***************************************************************************
                          rkprintagent  -  description
                             -------------------
    begin                : Mon Aug 01 2011
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

#ifndef RKPRINTAGENT_H
#define RKPRINTAGENT_H

#include <QObject>
#include <kparts/part.h>

/** The main purpose of this class is to cope with the lack of kprinter in KDE 4. Tries
 *  to offer a KDE print dialog for an existing postscript file. */
class RKPrintAgent : public QObject {
	Q_OBJECT
public:
	/** print the given postscript file.
	 *  @param delete_file : Try to delete the file after printing. Note: This is not guaranteed to work. */
	static void printPostscript (const QString &file, bool delete_file=false);
protected:
	RKPrintAgent ();
	~RKPrintAgent ();

	QString file;
	KParts::ReadOnlyPart *provider;
	bool delete_file;
};

#endif
