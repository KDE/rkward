/*
rkglobals - This file is part of the RKWard project. Created: Wed Aug 18 2004
SPDX-FileCopyrightText: 2004-2013 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef RKGLOBALS_H
#define RKGLOBALS_H

#include <QVariantMap>

class RKWardMainWindow;
class RInterface;
class RObjectList;
class RKModificationTracker;
class KHelpDlg;
class RControlWindow;
class QString;

/**
This class basically keeps some static pointers which are needed all over the place, so they won't have to be passed around.

TODO: move the static members to the respective classes instead. There's no point in having them here, and having to include rkglobals.h all over the place.

@author Thomas Friedrichsmeier
*/
class RKGlobals{
public:
/// returns KDialog::marginHint (), without the need to include kdialog.h in all the sources
	static int marginHint ();
/// returns KDialog::spacingHint (), without the need to include kdialog.h in all the sources
	static int spacingHint ();

	static QVariantMap startup_options;
};

#endif
