/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
include ("outliers_common.js");

function makeTestCall () {
	echo ('		t <- outlier (var, opposite = ' + getValue ("opposite") + ')\n');
	echo ('		results[i, \'Outlier\'] <- t\n');
}

function printout (is_preview) {
	if (!is_preview) {
		new Header (i18n ("Find potential outlier")).addFromUI ("opposite").print ();
	}
	echo ('rk.results (results)\n');
}


