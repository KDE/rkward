/*
rkquitagent - This file is part of the RKWard project. Created: Thu Jan 18 2007
SPDX-FileCopyrightText: 2007 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef RKQUITAGENT_H
#define RKQUITAGENT_H

#include <qobject.h>

class RKProgressControl;

/** The purpose of RKQuitAgent is to delay the actual destruction of the app until all commands have finished in the backend. The quit agent can NOT handle queries for saving some more data, or similar things. Do not call before you really want to quit the application.
@author Thomas Friedrichsmeier
*/
class RKQuitAgent : public QObject {
	Q_OBJECT
public:
/** Constructor. As soon as you construct an object of this type, the RKWard application *will* quit (but maybe with a short delay)! */
	explicit RKQuitAgent (QObject *parent);
	~RKQuitAgent ();

	static bool quittingInProgress () { return quitting; };
public Q_SLOTS:
	void doQuitNow ();
	void showWaitDialog ();
private:
	RKProgressControl *cancel_dialog;
	static bool quitting;
};

#endif
