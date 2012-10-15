// globals
var p;

function calculate () {
	p = "c (" + getString ("p").replace (/[, ]+/g, ", ") + ")";

	echo ('result <- (qwilcox (p = ' + p + ', m = ' + getValue ("m") + ', n = ' + getValue ("n") + ', ' + getValue ("tail") + ', ' + getValue("logp") + '))\n');
}

function printout () {
	echo ('rk.header ("Wilcoxon Rank Sum quantile", list ("Vector of probabilities", "' + p + '", "m (Numbers of observations in the first sample)", "' + getValue ("m") + '", "n (Numbers of observations in the second sample)", "' + getValue ("n") + '", "Tail", "' + getValue ("tail") + '", "Probabilities p are given as", "' + getValue ("logp") + '"))\n');
	echo ('rk.results (result, titles="Wilcoxon Rank Sum quantiles")\n');
}

