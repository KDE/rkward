/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Prasenjit Kapat <rkward-devel@kde.org>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
// globals
var nAvg;
var nDist;

include ('plot_clt_common.js');

function doParameters () {
	echo ('prob <- ' + getValue ("prob") + ';\n');
}

function doExpVar () {
	echo ('avg.exp <- (1-prob)/prob;\n');
	echo ('avg.var <- ((1-prob)/(prob^2))/' + nAvg + ';\n');
}

function doGenerateData () {
	echo ('data <- matrix(rgeom(n=' + nAvg*nDist + ', prob=prob), nrow=' + nAvg + ');\n');
}

