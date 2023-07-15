/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
function calculate () {
	if (getValue ("length.enabled.numeric")) {
		length = getValue ("length");
	} else {	// this happens when the saveto.parent is a data.frame, only.
		length = "dim (" + getValue ("saveto.parent") + ")[1]";
	}

	echo (".GlobalEnv$" + getValue ("saveto") + " <- rnorm (" + length + ", mean=" + getValue ("mean") + ", sd=" + getValue ("sd") + ")\n");
}
