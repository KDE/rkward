/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
function printout () {
	var x = getValue ("x");
	var scale = getValue ("scale");
	var width = getValue ("width");
	var atom = getValue ("atom");

	new Header (i18n ("Stem-and-Leaf Plot"))
		.add (i18n ("Variable"), noquote ('paste (rk.get.description (' + x + '))'))
		.addFromUI ("scale")
		.addFromUI ("width")
		.addFromUI ("atom").print ();
	echo ('\n');
	echo ('rk.print.literal(capture.output(stem(' + x + ', scale = ' + scale + ', width = ' + width + ', atom = ' + atom + ')))\n');
}

