// globals
var nAvg;
var nDist;

include ('plot_clt_common.js');

function doParameters () {
	echo ('df <- ' + getValue ("df") + '; ncp <- ' + getValue ("ncp") + ';\n');
}

function doExpVar () {
	if (getValue ("ncp") == 0) {
		echo ('avg.exp <- df;\n');
		echo ('avg.var <- (2*df)/' + nAvg + ';\n');
	} else {
		echo ('avg.exp <- df + ncp;\n');
		echo ('avg.var <- (2*df + 4*ncp)/' + nAvg + ';\n');
	}
}

function doGenerateData () {
	echo ('data <- matrix(rchisq(n=' + nAvg*nDist + ', df=df, ncp=ncp), nrow=' + nAvg + ');\n');
}

