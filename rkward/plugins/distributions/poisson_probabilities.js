// globals
var q;

function calculate () {
	q = "c (" + getString ("q").replace (/[, ]+/g, ", ") + ")";

	echo ('result <- (ppois (q = ' + q + ', lambda = ' + getValue ("lambda") + ', ' + getValue ("tail") + ', ' + getValue("logp") + '))\n');
}

function printout () {
	echo ('rk.header ("Poisson probability", list ("Vector of quantiles", "' + q + '", "Lambda", "' + getValue ("lambda") + '", "Tail", "' + getValue ("tail") + '", "Probabilities p are given as", "' + getValue ("logp") + '"))\n');
	echo ('rk.results (result, titles="Poisson probabilities")\n');
}

