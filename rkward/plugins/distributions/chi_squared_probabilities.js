// globals
var q;

function calculate () {
	q = "c (" + getValue ("q").replace (/[, ]+/g, ", ") + ")";

	echo ('result <- (pchisq (q = ' + q + ', df = ' + getValue ("df") + ',  ncp = ' + getValue ("ncp") + ', ' + getValue ("tail") + ', ' + getValue ("logp") + '))\n');
}

function printout () {
	echo ('rk.header ("Chi-squared probability", list ("Vector of quantiles", "' + q + '", "Degrees of Freedom", "' + getValue ("df") + '", "non-centrality parameter", "' + getValue ("ncp") + '", "Tail", "' + getValue ("tail") + '", "Probabilities p are given as", "' + getValue ("logp") + '"));\n');
	echo ('rk.results (result, titles="Chi-squared probabilities")\n');
}

