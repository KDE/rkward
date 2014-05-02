/***************************************************************************
                          showedittextfileagent  -  description
                             -------------------
    begin                : Tue Sep 13 2005
    copyright            : (C) 2005, 2010 by Thomas Friedrichsmeier
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

#ifndef SHOWEDITTEXTFILEAGENT_H
#define SHOWEDITTEXTFILEAGENT_H

#include <qobject.h>

class RBackendRequest;
class KDialog;

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
	KDialog *dialog;
};

#endif
