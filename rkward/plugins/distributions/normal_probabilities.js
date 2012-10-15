// globals
var q;

function calculate () {
	q = "c (" + getString ("q").replace (/[, ]+/g, ", ") + ")";

	echo ('result <- (pnorm (q = ' + q + ', mean = ' + getValue ("mean") + ', sd = ' + getValue ("sd") + ', ' + getValue ("tail") + ', ' + getValue ("logp") + '))\n');
}

function printout () {
	echo ('rk.header ("Normal probability", list ("Vector of quantiles", "' + q + '", "mu", "' + getValue ("mean") + '", "sigma", "' + getValue ("sd") + '", "Tail", "' + getValue ("tail") + '", "Probabilities p are given as", "' + getValue ("logp") + '"));\n');
	echo ('rk.results (result, titles="Normal probabilities")\n');
}

