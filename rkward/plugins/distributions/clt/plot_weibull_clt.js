/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Prasenjit Kapat <rkward-devel@kde.org>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
// globals
var nAvg;
var nDist;

include('plot_clt_common.js');

function doParameters() {
	echo('scale <- ' + getValue("scale") + '; shape <- ' + getValue("shape") + ';\n');
}

function doExpVar() {
	echo('avg.exp <- scale*gamma(1+1/shape);\n');
	echo('avg.var <- (scale^2*gamma(1+2/shape) - avg.exp^2)/' + nAvg + ';\n');
}

function doGenerateData() {
	echo('data <- matrix(rweibull(n=' + nAvg * nDist + ', shape=shape, scale=scale), nrow=' + nAvg + ');\n');
}
