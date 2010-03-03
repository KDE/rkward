// globals
var nAvg;
var nDist;

include ('plot_clt_common.js');

function doParameters () {
	echo ('loc <- ' + getValue ("loc") + '; scale <- ' + getValue ("scale") + ';\n');
}

function doExpVar () {
	echo ('avg.exp <- loc;\n');
	echo ('avg.var <- ((pi^2/3)*scale^2)/' + nAvg + ';\n');
}

function doGenerateData () {
	echo ('data <- matrix(rlogis(n=' + nAvg*nDist + ', location=loc, scale=scale), nrow=' + nAvg + ');\n');
}

