/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
function preprocess () {
	if (getValue ("is_embed")) {
		echo ('dev.set (' + getValue ("devnum") + ')\n');
	}
}

function printout () {
	var gridoptions = "";

	var nx = getValue ("nx");
	if (nx == "other") gridoptions = 'nx=' + getValue ("nx_cells");
	else gridoptions = 'nx=' + nx;

	var ny = getValue ("ny");
	if (ny == "other") gridoptions += ', ny=' + getValue ("ny_cells");
	else gridoptions += ', ny=' + ny;

	gridoptions += getValue ("col.code.printout");

	if (getValue("custlwd")) gridoptions += ', lwd=' + round(getValue ("lwd"),1);

	var lty = getValue("linetype");
	if (lty != "") gridoptions += ", lty=\"" + lty + "\"";

	if (!getValue("equilogs")) gridoptions += ', equilogs=FALSE';

	echo ('grid(' + gridoptions + ');\n');
}
