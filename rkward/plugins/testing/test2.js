/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
function preprocess () {
	echo ("This is\n\tan embedded\ntest");
}

function calculate () {
	echo ('embbeded: x: ' + getValue ("x") + 'y: ' + getValue ("y") + 'box: ' + getValue ("box"));
}
