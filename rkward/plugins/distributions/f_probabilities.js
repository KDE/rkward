// globals
var q;

function calculate () {
	q = "c (" + getString ("q").replace (/[, ]+/g, ", ") + ")";

	echo ('result <- (pf (q = ' + q + ', df1 = ' + getValue ("df1") + ', df2 = ' + getValue ("df2") + ',  ncp = ' + getValue ("ncp") + ', ' + getValue ("tail") + ', ' + getValue("logp") + '))\n');
}

function printout () {
	echo ('rk.header ("F probability", list ("Vector of quantiles", "' + q + '", "Numerator degrees of freedom", "' + getValue ("df1") + '", "Denominator degrees of freedom", "' + getValue ("df2") + '", "non-centrality parameter", "' + getValue ("ncp") + '", "Tail", "' + getValue ("tail") + '", "Probabilities p are given as", "' + getValue ("logp") + '"));\n');
	echo ('rk.results (result, titles="F probabilities")\n');
}

