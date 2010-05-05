/***************************************************************************
                          rkjobsequence  -  description
                             -------------------
    begin                : Tue May 04
    copyright            : (C) 2010 by Thomas Friedrichsmeier
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

#ifndef RKJOBSEQUENCE_H
#define RKJOBSEQUENCE_H

#include <QStringList>

#include <kjob.h>

/** Simple class to queue up a sequnce of KJob that will be executed one after the other */
class RKJobSequence : public QObject {
	Q_OBJECT
public:
	RKJobSequence ();
	~RKJobSequence ();

	void addJob (KJob* job);
	bool hadError () const;
	QStringList errors () const;
	void start ();
private slots:
	void jobDone (KJob* job);
signals:
	void finished (RKJobSequence *seq);
private:
	void nextJob ();

	QList<KJob*> outstanding_jobs;
	QStringList _errors;
};

#endif