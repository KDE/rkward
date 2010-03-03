// globals
var nAvg;
var nDist;

include ('plot_clt_common.js');

function doParameters () {
	echo ('shape <- ' + getValue ("shape") + '; rate <- ' + getValue ("rate") + ';\n');
}

function doExpVar () {
	echo ('avg.exp <- shape/rate;\n');
	echo ('avg.var <- (shape/(rate^2))/' + nAvg + ';\n');
}

function doGenerateData () {
	echo ('data <- matrix(rgamma(n=' + nAvg*nDist + ', shape=shape, rate=rate), nrow=' + nAvg + ');\n');
}

