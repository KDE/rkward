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

