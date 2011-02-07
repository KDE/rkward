/***************************************************************************
                          rkrecoverdialog  -  description
                             -------------------
    begin                : Fri Feb 04 2011
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

#ifndef RKRECOVERDIALOG_H
#define RKRECOVERDIALOG_H

#include <kdialog.h>

#include <QStringList>

/** Dialog to offer loading of recovery files during startup. */
class RKRecoverDialog : public KDialog {
	Q_OBJECT
public:
/** Check whether a crash recovery file is available. If so, display a dialog, offering to load the recovery file.
@returns The url of the recovery file, if user selected to load it. An empty KUrl otherwise. */
	static KUrl checkRecoverCrashedWorkspace ();
protected:
	RKRecoverDialog (const QStringList &recovery_files);
	~RKRecoverDialog ();
private slots:
	void showButtonClicked ();
};

#endif
