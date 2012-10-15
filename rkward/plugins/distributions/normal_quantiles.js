// globals
var p;

function calculate () {
	p = "c (" + getString ("p").replace (/[, ]+/g, ", ") + ")";

	echo ('result <- (qnorm (p = ' + p + ', mean = ' + getValue ("mean") + ', sd = ' + getValue ("sd") + ', ' + getValue ("tail") + ', ' + getValue ("logp") + '))\n');
}

function printout () {
	echo ('rk.header ("Normal quantile", list ("Vector of probabilities", "' + p + '", "mu", "' + getValue ("mean") + '", "sigma", "' + getValue ("sd") + '", "Tail", "' + getValue ("tail") + '", "Probabilities p are given as", "' + getValue ("logp") + '"));\n');
	echo ('rk.results (result, titles="Normal quantiles")\n');
}

