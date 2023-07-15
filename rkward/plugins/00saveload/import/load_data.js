/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
// globals
var envir;

function calculate () {
	var other_env = false;
	if (getValue ("envir.active")) {
		other_env = true;
		envir = ".GlobalEnv$" + getValue ("envir");
	} else {
		envir = "globalenv()";
	}

	if (other_env) {
		echo (envir + ' <- new.env (parent=globalenv())\n');
	}
	echo ('load (file="' + getValue("file") + '", envir=' + envir + ')\n');
}

function printout () {
	new Header (i18n ("Load data")).addFromUI ("file").add (i18n ("Import to environment"), envir).print ();
}
