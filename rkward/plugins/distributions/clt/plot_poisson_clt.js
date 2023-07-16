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
	echo ('mean <- ' + getValue ("mean") + ';\n');
}

function doExpVar () {
	echo ('avg.exp <- mean;\n');
	echo ('avg.var <- (mean)/' + nAvg + ';\n');
}

function doGenerateData () {
	echo ('data <- matrix(rpois(n=' + nAvg*nDist + ', lambda=mean), nrow=' + nAvg + ');\n');
}

