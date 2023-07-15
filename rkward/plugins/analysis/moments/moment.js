/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
include ("moments_common.js");

function insertTestCall () {
	echo ('		results[i, ' + i18nc ("Statistical moment", "Moment") + '] <- moment (var, order = ' + getValue ("order") + ', central = ' + getValue ("central") + ', absolute = ' + getValue ("absolute") + ', na.rm = ' + getValue ("narm") + ')\n');
}

function printout (is_preview) {
	if (!is_preview) {
		new Header (i18n ("Statistical Moment")).addFromUI ("order").addFromUI ("central").addFromUI ("absolute").addFromUI ("narm").print ();
	}
	echo ('rk.results (results)\n');
}

