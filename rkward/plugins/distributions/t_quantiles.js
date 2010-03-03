// globals
var p;

function calculate () {
	p = "c (" + getValue ("p").replace (/[, ]+/g, ", ") + ")";

	echo ('result <- (qt (p = ' + p + ', df = ' + getValue ("df") + ', ncp = ' + getValue ("ncp") + ', ' + getValue ("tail") + ', ' + getValue ("logp") + '))\n');
}

function printout () {
	echo ('rk.header ("t quantile", list ("Vector of probabilities", "' + p + '", "Degrees of freedom", "' + getValue ("df") + '", "non-centrality parameter", "' + getValue ("ncp") + '", "Tail", "' + getValue ("tail") + '", "Probabilities p are given as", "' + getValue ("logp") + '"));\n');
	echo ('rk.results (result, titles="t quantiles")\n');
}

