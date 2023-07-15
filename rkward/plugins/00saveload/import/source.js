/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
function calculate () {
	var prompt = "";
	// most options should only be shown, if they differ from the default.
	var options = "";
	if (getValue ("echo")) {
		options += ", echo=TRUE";
		var prompt = getValue ("promptecho");
		if (prompt.length > 0) {
			options += ", prompt.echo=\"" + prompt + "\"";
		}
		options += ", max.deparse.length=" + getValue ("maxdeparselength");
		options += ", verbose=" + getValue ("verbose");
	} else {
		options += ", verbose=FALSE";
	}
	options += ", print.eval=" + getValue ("printeval");

	echo ('source (file="' + getValue("file") + '", local=' + getValue("local") + options + ', chdir=' + getValue("chdir") + ')\n');
}

function printout () {
	new Header (i18n ("Source R file")).addFromUI ("file").print ();
}


