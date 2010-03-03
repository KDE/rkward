// globals
var nAvg;
var nDist;

include ('plot_clt_common.js');

function doParameters () {
	echo ('mean <- ' + getValue ("mean") + '; sd <- ' + getValue ("sd") + ';\n');
}

function doExpVar () {
	echo ('avg.exp <- mean;\n');
	echo ('avg.var <- (sd^2)/' + nAvg + ';\n');
}

function doGenerateData () {
	echo ('data <- matrix(rnorm(n=' + nAvg*nDist + ', mean=mean, sd=sd), nrow=' + nAvg + ');\n');
}

