/*
rkprintagent - This file is part of the RKWard project. Created: Mon Aug 01 2011
SPDX-FileCopyrightText: 2011 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKPRINTAGENT_H
#define RKPRINTAGENT_H

#include <QObject>
#include <kparts/readonlypart.h>

/** The main purpose of this class is to cope with the lack of kprinter in KDE 4. Tries
 *  to offer a KDE print dialog for an existing postscript file. */
class RKPrintAgent : public QObject {
	Q_OBJECT
public:
	/** print the given postscript file.
	 *  @param delete_file : Try to delete the file after printing. Note: This is not guaranteed to work. */
	static void printPostscript (const QString &file, bool delete_file=false);
protected:
	RKPrintAgent(const QString &file, KParts::ReadOnlyPart *provider, bool delete_file);
	~RKPrintAgent();

	QString file;
	KParts::ReadOnlyPart *provider;
	bool delete_file;
};

#endif
