// globals
var q;

function calculate () {
	q = "c (" + getString ("q").replace (/[, ]+/g, ", ") + ")";

	echo ('result <- (phyper (q = ' + q + ', m = ' + getValue ("m") + ', n = ' + getValue ("n") + ', k = ' + getValue ("k") + ', ' + getValue ("tail") + ', ' + getValue("logp") + '))\n');
}

function printout () {
	echo ('rk.header ("Hypergeometric probability", list ("Vector of quantiles", "' + q + '", "Number of white balls in the urn", "' + getValue ("m") + '", "Number of black balls in the urn", "' + getValue ("n") + '", "Number of balls drawn from the urn", "' + getValue ("k") + '", "Tail", "' + getValue ("tail") + '", "Probabilities p are given as", "' + getValue ("logp") + '"))\n');
	echo ('rk.results (result, titles="Hypergeometric probabilities")\n');
}

