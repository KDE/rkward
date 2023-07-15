/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
/* NOTE: This file is currently not used by the color_chooser plugin.
 Pehaps we can start using it again, now that a full PHP backend no longer needs to be started for this simple plugin. TODO: test it.
*/

function printout () {
	var col = getValue ("color");
	if (empty (col)) col = getValue ("default_color");
	if (!empty (col)) col = getValue ("argument") + "\"" + col + "\"";

	echo (col);
}
