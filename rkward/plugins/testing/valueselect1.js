/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
function calculate () {
	echo (getList ("select").join ("\n"));
	echo ("\n\n");
	echo (getList ("valueslot").join ("\n"));
	echo ("\n\n");
}

