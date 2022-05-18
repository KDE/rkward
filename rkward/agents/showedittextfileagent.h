/*
showedittextfileagent - This file is part of the RKWard project. Created: Tue Sep 13 2005
SPDX-FileCopyrightText: 2005-2016 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SHOWEDITTEXTFILEAGENT_H
#define SHOWEDITTEXTFILEAGENT_H

#include <qobject.h>

class RBackendRequest;
class KMessageWidget;

/** The purpose of this agent is to display text files for showing and/or editing on request of the R backend. Technically speaking, it gets invoked, whenever the standard R callbacks (ptr_)R_ShowFiles, (ptr_)R_EditFiles, or (ptr_)R_EditFile are called. While the task of simply opening such files for display/editing is rather simple, there is one slightly more difficult issue involved (and hence this class to handle it): In standard R, all these calls are blocking further processing, until the user has closed the shown/edited files. In some cases this may be necessary (for instance if R wants to use with the edited files immediately), in some cases this is an unnecessary nuisance (such as when R simply wants to display a help page or some other purely informational text).

The solution to this, used in this agent, is to display a non-modal dialog with a "Done"-button, and thereby leave the decision to the user. Whenever the user thinks, it's safe to resume processing, the "Done"-button can be pressed, even if the files are still open. Until that time, processing is halted.

You probably don't want to create an instance of this agent directly. Rather use the static showEditFiles ().

@author Thomas Friedrichsmeier
*/
class ShowEditTextFileAgent : public QObject {
public:
/** constructor. Do not use directly. Use the static showEditFiles () instead.
@param args The corresponding RCallbackArgs-record
@param text Text to display in the dialog. */
	ShowEditTextFileAgent (RBackendRequest *request, const QString &text, const QString &caption);

/** destructor. Called when the user presses the done button (or closes the notification dialog). Resumes processing in the backend, deletes the agent */
	~ShowEditTextFileAgent ();

/** This gets called by RInterface, in order to show/edit the files in question. The RCallbackArgs-struct is passed in raw form, and only this function sorts
out, what exactly needs to be done. Note that this is a static member. It will take care of creating/deleting an agent on its own. */
	static void showEditFiles (RBackendRequest *request);
private:
	RBackendRequest *request;
	KMessageWidget *message;
};

#endif
