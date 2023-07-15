/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
function preprocess () {
	echo ('require(car)\n');
}


function printout () {
	doPrintout (true);
}

function preview () {
	preprocess ();
	doPrintout (false);
}

function doPrintout (full) {
	var vars = getList ("x").join (",");

	echo ('data <- data.frame (' + vars + ')\n');
	echo ('\n');
	if (full) {
		new Header (i18n ("Scatterplot Matrix")).addFromUI ("diag").addFromUI ("plot_points").addFromUI ("smooth").addFromUI ("ellipse").print ();
		echo ('\n');
		echo ('rk.graph.on ()\n');
	}
	echo ('try (scatterplotMatrix(data, diagonal=' + getValue("diag") + ', plot.points=' + getValue ("plot_points") + ', smooth=' + getValue ("smooth") + ', ellipse=' + getValue ("ellipse") + '))\n');
	if (full) {
		echo ('rk.graph.off ()\n');
	}
}

