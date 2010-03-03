// globals
var q;

function calculate () {
	q = "c (" + getValue ("q").replace (/[, ]+/g, ", ") + ")";

	echo ('result <- (pgamma (q = ' + q + ', shape = ' + getValue ("shape") + ', rate = ' + getValue ("rate") + ', ' + getValue ("tail") + ', ' + getValue("logp") + '))\n');
}

function printout () {
	echo ('rk.header ("Gamma probability", list ("Vector of quantiles", "' + q + '", "Shape", "' + getValue ("shape") + '", "Rate", "' + getValue ("rate") + '", "Tail", "' + getValue ("tail") + '", "Probabilities p are given as", "' + getValue ("logp") + '"))\n');
	echo ('rk.results (result, titles="Gamma probabilities")\n');
}

