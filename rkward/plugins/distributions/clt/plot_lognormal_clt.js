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
	echo ('mean <- ' + getValue ("mean") + '; sd <- ' + getValue ("sd") + ';\n');
}

function doExpVar () {
	echo ('avg.exp <- exp(mean+sd^2/2);\n');
	echo ('avg.var <- (exp(2*mean+sd^2)*(exp(sd^2)-1))/' + nAvg + ';\n');
}

function doGenerateData () {
	echo ('data <- matrix(rlnorm(n=' + nAvg*nDist + ', meanlog=mean, sdlog=sd), nrow=' + nAvg + ');\n');
}

