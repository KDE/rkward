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
	echo ('llim <- ' + getValue ("llim") + '; ulim <- ' + getValue ("ulim") + ';\n');
}

function doExpVar () {
	echo ('avg.exp <- (llim+ulim)/2;\n');
	echo ('avg.var <- ((ulim-llim)^2/12)/' + nAvg + ';\n');
}

function doGenerateData () {
	echo ('data <- matrix(runif(n=' + nAvg*nDist + ', min=llim, max=ulim), nrow=' + nAvg + ');\n');
}

