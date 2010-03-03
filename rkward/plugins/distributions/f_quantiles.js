// globals
var p;

function calculate () {
	p = "c (" + getValue ("p").replace (/[, ]+/g, ", ") + ")";

	echo ('result <- (qf (p = ' + p + ', df1 = ' + getValue ("df1") + ', df2 = ' + getValue ("df2") + ', ncp = ' + getValue ("ncp") + ', ' + getValue ("tail") + ', ' + getValue("logp") + '))\n');
}

function printout () {
	echo ('rk.header ("F quantile", list ("Vector of probabilities", "' + p + '", "Numerator degrees of freedom", "' + getValue ("df1") + '", "Denominator degrees of freedom", "' + getValue ("df2") + '", "non-centrality parameter", "' + getValue ("ncp") + '", "Tail", "' + getValue ("tail") + '", "Probabilities p are given as", "' + getValue ("logp") + '"));\n');
	echo ('rk.results (result, titles="F quantiles")\n');
}

