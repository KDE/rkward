// globals
var p;

function calculate () {
	p = "c (" + getString ("p").replace (/[, ]+/g, ", ") + ")";

	echo ('result <- (qgamma (p = ' + p + ', shape = ' + getValue ("shape") + ', rate = ' + getValue ("rate") + ', ' + getValue ("tail") + ', ' + getValue("logp") + '))\n');
	echo ('\n');
}

function printout () {
	echo ('rk.header ("Gamma quantile", list ("Vector of probabilities", "' + p + '", "Shape", "' + getValue ("shape") + '", "Rate", "' + getValue ("rate") + '", "Tail", "' + getValue ("tail") + '", "Probabilities p are given as", "' + getValue ("logp") + '"))\n');
	echo ('rk.results (result, titles="Gamma quantiles")\n');
}

