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
	echo ('m <- ' + getValue ("nm") + '; n <- ' + getValue ("nn") + ';\n');
}

function doExpVar () {
	echo ('avg.exp <- m*n/2;\n');
	echo ('avg.var <- (m*n*(m+n+1)/12)/' + nAvg + ';\n');
}

function doGenerateData () {
	echo ('data <- matrix(rwilcox(nn=' + nAvg*nDist + ', m=m, n=n), nrow=' + nAvg + ');\n');
}

