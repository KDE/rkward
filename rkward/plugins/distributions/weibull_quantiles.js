// globals
var p;

function calculate () {
	p = "c (" + getString ("p").replace (/[, ]+/g, ", ") + ")";

	echo ('result <- (qweibull (p = ' + p + ', shape = ' + getValue ("shape") + ', scale = ' + getValue ("scale") + ', ' + getValue ("tail") + ', ' + getValue("logp") + '))\n');
}

function printout () {
	echo ('rk.header ("Weibull quantile", list ("Vector of probabilities", "' + p + '", "Shape", "' + getValue ("shape") + '", "Scale", "' + getValue ("scale") + '", "Tail", "' + getValue ("tail") + '", "Probabilities p are given as", "' + getValue ("logp") + '"))\n');
	echo ('rk.results (result, titles="Weibull quantiles")\n');
}

