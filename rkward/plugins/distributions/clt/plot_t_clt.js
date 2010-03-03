// globals
var nAvg;
var nDist;

include ('plot_clt_common.js');

function doParameters () {
	echo ('df <- ' + getValue ("df") + '; ncp <- ' + getValue ("ncp") + ';\n');
}

function doExpVar () {
	if (getValue ("ncp") == 0) {
		echo ('avg.exp <- 0;\n');
		echo ('avg.var <- df/((df-2)*' + nAvg + ');\n');
	} else {
		echo ('tmp.var <- gamma((df-1)/2)/gamma(df/2);\n');
		echo ('avg.exp <- ncp*sqrt(df/2)*tmp.var;\n');
		echo ('avg.var <- (df*(1+ncp^2)/(df-2) - ncp^2*df*tmp.var^2/2)/' + nAvg + ';\n');
	}
}

function doGenerateData () {
	echo ('data <- matrix(rt(n=' + nAvg*nDist + ', df=df, ncp=ncp), nrow=' + nAvg + ');\n');
}

