// globals
var nAvg;
var nDist;

include ('plot_clt_common.js');

function doParameters () {
	echo ('size <- ' + getValue ("size") + '; prob <- ' + getValue ("prob") + ';\n');
}

function doExpVar () {
	echo ('avg.exp <- size*prob;\n');
	echo ('avg.var <- (size*prob*(1-prob))/' + nAvg + ';\n');
}

function doGenerateData () {
	echo ('data <- matrix(rbinom(n=' + nAvg*nDist + ', size=size, prob=prob), nrow=' + nAvg + ');\n');
}

