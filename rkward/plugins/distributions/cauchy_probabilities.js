// globals
var q;

function calculate () {
	q = "c (" + getString ("q").replace (/[, ]+/g, ", ") + ")";

	echo ('result <- (pcauchy (q = ' + q + ', location = ' + getValue ("location") + ', scale = ' + getValue ("scale") + ', ' + getValue ("tail") + ', ' + getValue("logp") + '))\n');
}

function printout () {
	echo ('rk.header ("Cauchy probabilities", list ("Vector of quantiles", "' + q + '", "Location", "' + getValue ("location") + '", "Scale", "' + getValue ("scale") + '", "Tail", "' + getValue ("tail") + '", "Probabilities p are given as", "' + getValue ("logp") + '"))\n');
	echo ('rk.results (result, titles="Cauchy probabilities")\n');
}

