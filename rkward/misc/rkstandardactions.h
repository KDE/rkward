/*
rkstandardactions - This file is part of the RKWard project. Created: Sun Nov 18 2007
SPDX-FileCopyrightText: 2007-2016 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RKSTANDARDACTIONS_H
#define RKSTANDARDACTIONS_H

class QAction;
class QString;
class QObject;
class RKMDIWindow;
class RKScriptContextProvider;

/** This namespace provides functions to generate some standard actions, i.e. actions which are needed at more than one place.

@author Thomas Friedrichsmeier */
namespace RKStandardActions {
	QAction *copyLinesToOutput (RKMDIWindow *window, const QObject *receiver=nullptr, const char *member=nullptr);
/** Allows special pasting modes for script windows.
@param member needs to have the signature void fun (const QString&). */
	QAction* pasteSpecial (RKMDIWindow *window, const QObject *receiver=nullptr, const char *member=nullptr);

	QAction* runCurrent (RKMDIWindow *window, const QObject *receiver=nullptr, const char *member=nullptr, bool current_or_line=false);
	QAction* runAll (RKMDIWindow *window, const QObject *receiver=nullptr, const char *member=nullptr);

	QAction* functionHelp (RKMDIWindow *window, RKScriptContextProvider *context_provider);
/** Search for current symbol / selection, online. Note that you will not have to connect this action to any slot to work. It does everything by itself.
 *  It will query the given context_provider for context. */
	QAction* onlineHelp (RKMDIWindow *window, RKScriptContextProvider *context_provider);
};

#endif
