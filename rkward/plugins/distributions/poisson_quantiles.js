// globals
var p;

function calculate () {
	p = "c (" + getValue ("p").replace (/[, ]+/g, ", ") + ")";

	echo ('result <- (qpois (p = ' + p + ', lambda = ' + getValue ("lambda") + ', ' + getValue ("tail") + ', ' + getValue("logp") + '))\n');
}

function printout () {
	echo ('rk.header ("Poisson quantile", list ("Vector of probabilities", "' + p + '", "Lambda", "' + getValue ("lambda") + '", "Tail", "' + getValue ("tail") + '", "Probabilities p are given as", "' + getValue ("logp") + '"))\n');
	echo ('rk.results (result, titles="Poisson quantiles")\n');
}

