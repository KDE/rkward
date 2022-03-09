/***************************************************************************
                          rksaveagent  -  description
                             -------------------
    begin                : Sun Aug 29 2004
    copyright            : (C) 2004-2020 by Thomas Friedrichsmeier
    email                : thomas.friedrichsmeier@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef RKSAVEAGENT_H
#define RKSAVEAGENT_H

#include "../rbackend/rcommandreceiver.h"

#include <QUrl>

class RCommandChain;

/**
This class used to have much more (over-)complexity. It could probably be merged into RKWorkplace, today. Used to control workspace saving.

@author Thomas Friedrichsmeier
*/
class RKSaveAgent {
public:
/** Save the workspace. If no URL is given use the last known save url. If the workspace has not been saved, previously, ask for url to save to. */
	static bool saveWorkspace(const QUrl &url=QUrl());
/** Save the workspace, asking for a (new) file name.
 @param previous_url If given, specified the default directory and file name. */
	static bool saveWorkspaceAs(const QUrl &previous_url=QUrl());
};

#endif
