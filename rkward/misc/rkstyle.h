/*
rkstyle - This file is part of the RKWard project. Created: Wed Apr 13 2022
SPDX-FileCopyrightText: 2022 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef RKSTYLE_H
#define RKSTYLE_H

class KColorScheme;

/**
Namespace to hold common styling options: Spacing and color scheme.

@author Thomas Friedrichsmeier
*/
class RKStyle{
public:
/// @returns KDialog::marginHint (), without the need to include kdialog.h in all the sources
	static int marginHint ();
/// @returns KDialog::spacingHint (), without the need to include kdialog.h in all the sources
	static int spacingHint ();
/// @returns a cached instance of the color scheme for normal views. Particularly useful for setting specials colors in item models
	static KColorScheme* viewScheme();
private:
friend class RKWardMainWindow;
	static KColorScheme* _view_scheme;
};

#endif
