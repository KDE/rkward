// globals
var nAvg;
var nDist;

include ('plot_clt_common.js');

function doParameters () {
	echo ('prob <- ' + getValue ("prob") + ';\n');
}

function doExpVar () {
	echo ('avg.exp <- (1-prob)/prob;\n');
	echo ('avg.var <- ((1-prob)/(prob^2))/' + nAvg + ';\n');
}

function doGenerateData () {
	echo ('data <- matrix(rgeom(n=' + nAvg*nDist + ', prob=prob), nrow=' + nAvg + ');\n');
}

