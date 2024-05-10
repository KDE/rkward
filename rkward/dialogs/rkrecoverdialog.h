/*
rkrecoverdialog - This file is part of the RKWard project. Created: Fri Feb 04 2011
SPDX-FileCopyrightText: 2011-2014 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKRECOVERDIALOG_H
#define RKRECOVERDIALOG_H

#include <QDialog>
#include <QStringList>

/** Dialog to offer loading of recovery files during startup. */
class RKRecoverDialog : public QDialog {
	Q_OBJECT
public:
/** Check whether a crash recovery file is available. If so, display a dialog, offering to load the recovery file.
@returns The url of the recovery file, if user selected to load it. An empty QUrl otherwise. */
	static QUrl checkRecoverCrashedWorkspace ();
protected:
	explicit RKRecoverDialog(const QStringList &recovery_files);
	~RKRecoverDialog();
	static QString saveFileFor (const QString &recovery_file);
	QStringList files;
private Q_SLOTS:
	void deleteButtonClicked ();
};

#endif
