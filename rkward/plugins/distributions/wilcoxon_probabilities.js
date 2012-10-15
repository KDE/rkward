// globals
var q;

function calculate () {
	q = "c (" + getString ("q").replace (/[, ]+/g, ", ") + ")";

	echo ('result <- (pwilcox (q = ' + q + ', m = ' + getValue ("m") + ', n = ' + getValue ("n") + ', ' + getValue ("tail") + ', ' + getValue("logp") + '))\n');
}

function printout () {
	echo ('rk.header ("Wilcoxon Rank Sum probability", list ("Vector of quantiles", "' + q + '", "m (Numbers of observations in the first sample)", "' + getValue ("m") + '", "n (Numbers of observations in the second sample)", "' + getValue ("n") + '", "Tail", "' + getValue ("tail") + '", "Probabilities p are given as", "' + getValue ("logp") + '"))\n');
	echo ('rk.results (result, titles="Wilcoxon Rank Sum probabilities")\n');
}

