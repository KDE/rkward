// globals
var q;

function calculate () {
	q = "c (" + getString ("q").replace (/[, ]+/g, ", ") + ")";

	echo ('result <- (plnorm (q = ' + q + ', meanlog = ' + getValue ("meanlog") + ', sdlog = ' + getValue ("sdlog") + ', ' + getValue ("tail") + ', ' + getValue ("logp") + '))\n');
}

function printout () {
	echo ('rk.header ("Log Normal probability", list ("Vector of quantiles", "' + q + '", "meanlog", "' + getValue ("meanlog") + '", "sdlog", "' + getValue ("sdlog") + '", "Tail", "' + getValue ("tail") + '", "Probabilities p are given as", "' + getValue ("logp") + '"))\n');
	echo ('rk.results (result, titles="Log Normal probabilities")\n');
}

