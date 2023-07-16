/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
function calculate () {
	var vars = trim (getValue ("data")).replace (/\n/g, "','");

	echo ('package.skeleton(name="' + getValue("name") + '", list=c(\'' + vars + '\'), path="' + getValue("path") + '", force= ' + getValue("force") + ')\n');
}

function printout () {
	new Header (i18n ("Create package skeleton")).addFromUI ("name").addFromUI ("path").print ();
}

