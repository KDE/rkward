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
	echo ('m <- ' + getValue ("m") + '; n <- ' + getValue ("n") + '; k <- ' + getValue ("k") + ';\n');
}

function doExpVar () {
	echo ('avg.exp <- k*m/(m+n);\n');
	echo ('avg.var <- (k*m*n*(m+n-k)/((m+n)^2*(m+n-1)))/' + nAvg + ';\n');
}

function doGenerateData () {
	echo ('data <- matrix(rhyper(nn=' + nAvg*nDist + ', m=m, n=n, k=k), nrow=' + nAvg + ');\n');
}

