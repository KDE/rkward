/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
function calculate () {
	echo ("Matrix A:\n");
	echo (getValue ("matrixa.cbind"));
	echo ("\nMatrix B:\n");
	echo (getValue ("matrixb.cbind"));
	echo ("\nMatrix C:\n");
	echo (getValue ("matrixc.cbind"));
	echo ("\nMatrix C, column 2:\n");
	echo (getValue ("matrixc.2"));
}

