/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
function preprocess () {
	var vars = trim (getValue ("x")).replace (/\n/g, ",");

	echo ('data <- data.frame (rk.list(' + vars + '), check.names=FALSE)\n');
}

function calculate () {
	echo ('result <- ftable (data');

	var rows = getValue("rows");
	if (rows == 0) echo(', row.vars=c()');
	else echo(', row.vars=1:min(' + rows + ', length(data))');

	if (!getValue ("exclude_nas.state")) echo (", exclude=NULL");
	echo (');\n');
}

function printout(is_preview) {
	if (!is_preview) {
		new Header (i18n ("Crosstabs (n to n)")).add (i18n ("Variables"), noquote ('paste(names(data), collapse=", ")')).print ();
		echo ('\n');
	}
	echo ('rk.print (result)\n');
}
