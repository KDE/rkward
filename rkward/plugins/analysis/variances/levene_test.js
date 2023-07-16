/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
function preprocess () {
	echo ('require(car)\n');
}

function calculate () {
	echo ('result <- leveneTest (' + getValue ("y") + ', ' + getValue ("group") + ')\n');
}

function printout (is_preview) {
	echo ('names <- rk.get.description (' + getValue ("y") + ', ' + getValue ("group") + ')\n');
	echo ('\n');
	if (!is_preview) {
		new Header (i18n ("Levene's Test")).addFromUI ("y", noquote ('names[1]')).addFromUI ("group", noquote ('names[2]')).print ();
		echo ('\n');
	}
	echo ('rk.print (result)\n');
}

