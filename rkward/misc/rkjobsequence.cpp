/*
rkjobsequence - This file is part of RKWard (https://rkward.kde.org). Created: Tue May 04
SPDX-FileCopyrightText: 2010 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
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
	connect (job, &KJob::result, this, &RKJobSequence::jobDone);
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
		Q_EMIT finished(this);
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

