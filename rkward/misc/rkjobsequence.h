/*
rkjobsequence - This file is part of the RKWard project. Created: Tue May 04
SPDX-FileCopyrightText: 2010 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKJOBSEQUENCE_H
#define RKJOBSEQUENCE_H

#include <QStringList>

#include <kjob.h>

/** Simple class to queue up a sequence of KJob that will be executed one after the other */
class RKJobSequence : public QObject {
	Q_OBJECT
public:
	RKJobSequence ();
	~RKJobSequence ();

	void addJob (KJob* job);
	bool hadError () const;
	QStringList errors () const;
	void start ();
private Q_SLOTS:
	void jobDone (KJob* job);
Q_SIGNALS:
	void finished (RKJobSequence *seq);
private:
	void nextJob ();

	QList<KJob*> outstanding_jobs;
	QStringList _errors;
};

#endif
