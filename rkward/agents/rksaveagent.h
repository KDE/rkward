/*
rksaveagent - This file is part of the RKWard project. Created: Sun Aug 29 2004
SPDX-FileCopyrightText: 2004-2020 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef RKSAVEAGENT_H
#define RKSAVEAGENT_H

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
