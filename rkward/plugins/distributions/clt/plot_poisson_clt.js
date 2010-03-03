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

