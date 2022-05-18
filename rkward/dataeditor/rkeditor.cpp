/*
rkeditor - This file is part of RKWard (https://rkward.kde.org). Created: Fri Aug 20 2004
SPDX-FileCopyrightText: 2004 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "rkeditor.h"

#include "../debug.h"

RKEditor::RKEditor (QWidget *parent) : RKMDIWindow (parent, RKMDIWindow::DataEditorWindow) {
	RK_TRACE (EDITOR);
}

RKEditor::~RKEditor () {
	RK_TRACE (EDITOR);
//	getObject ()->setObjectOpened (this, false);
}

