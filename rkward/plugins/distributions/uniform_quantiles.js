// globals
var p;

function calculate () {
	p = "c (" + getString ("p").replace (/[, ]+/g, ", ") + ")";

	echo ('result <- (qunif (p = ' + p + ', min = ' + getValue ("min") + ', max = ' + getValue ("max") + ', ' + getValue ("tail") + ', ' + getValue("logp") + '))\n');
}

function printout () {
	echo ('rk.header ("Uniform quantile", list ("Vector of probabilities", "' + p + '", "Lower limits of the distribution", "' + getValue ("min") + '", "Upper limits of the distribution", "' + getValue ("max") + '", "Tail", "' + getValue ("tail") + '", "Probabilities p are given as", "' + getValue ("logp") + '"))\n');
	echo ('rk.results (result, titles="Uniform quantiles")\n');
}

