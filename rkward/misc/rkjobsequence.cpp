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
#include "rkjobsequence.h"

#include "../debug.h"

RKJobSequence::RKJobSequence () : QObject () {
	RK_TRACE (MISC);
}

RKJobSequence::~RKJobSequence () {
	RK_TRACE (MISC);
}

void RKJobSequence::addJob (KJob* job) {
	RK_TRACE (MISC);

	outstanding_jobs.append (job);
	connect (job, SIGNAL (result(KJob*)), this, SLOT(jobDone(KJob*)));
}

bool RKJobSequence::hadError () const {
	RK_TRACE (MISC);

	return (!_errors.isEmpty ());
}

QStringList RKJobSequence::errors () const {
	RK_TRACE (MISC);

	return (_errors);
}

void RKJobSequence::start () {
	RK_TRACE (MISC);

	nextJob ();
}

void RKJobSequence::nextJob () {
	RK_TRACE (MISC);

	if (outstanding_jobs.isEmpty ()) {
		emit (finished (this));
		deleteLater ();
		return;
	}

	outstanding_jobs.first ()->start ();
}

void RKJobSequence::jobDone (KJob* job) {
	RK_TRACE (MISC);

	outstanding_jobs.removeAll (job);
	if (job->error ()) {
		_errors.append (job->errorString ());
	}
	nextJob ();
}

#include "rkjobsequence.moc"
