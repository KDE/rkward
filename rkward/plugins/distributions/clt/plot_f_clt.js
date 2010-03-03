// globals
var nAvg;
var nDist;

include ('plot_clt_common.js');

function doParameters () {
	echo ('df1 <- ' + getValue ("df1") + '; df2 <- ' + getValue ("df2") + '; ncp <- ' + getValue ("ncp") + ';\n');
}

function doExpVar () {
	if (getValue ("ncp") == 0) {
		echo ('avg.exp <- df2*df1/(df1*(df2-2));\n');
		echo ('avg.var <- (2*df2^2*(df1+df2-2)/(df1*(df2-2)^2*(df2-4)))/' + nAvg + ';\n');
	} else {
		echo ('avg.exp <- df2*(df1+ncp)/(df1*(df2-2));\n');
		echo ('avg.var <- (2*df2^2*((df1+ncp)^2  + (df1+2*ncp)*(df2-2)) / (df1^2*(df2-2)^2*(df2-4)))/' + nAvg + ';\n');
	}
}

function doGenerateData () {
	echo ('data <- matrix(rf(n=' + nAvg*nDist + ', df1=df1, df2=df2, ncp=ncp), nrow=' + nAvg + ');\n');
}

