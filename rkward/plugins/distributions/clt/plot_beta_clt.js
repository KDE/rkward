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
	echo ('shape1 <- ' + getValue ("a") + '; shape2 <- ' + getValue ("b") + ';\n');
}

function doExpVar () {
	echo ('avg.exp <- shape1/(shape1 + shape2);\n');
	echo ('avg.var <- (shape1*shape2/((shape1+shape2)^2*(shape1+shape2+1)))/' + nAvg + ';\n');
}

function doGenerateData () {
	echo ('data <- matrix(rbeta(n=' + nAvg*nDist + ', shape1=shape1, shape2=shape2), nrow=' + nAvg + ');\n');
}

