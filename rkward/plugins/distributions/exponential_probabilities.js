// globals
var q;

function calculate () {
	q = "c (" + getValue ("q").replace (/[, ]+/g, ", ") + ")";

	echo ('result <- (pexp (q = ' + q + ', rate = ' + getValue ("rate") + ', ' + getValue ("tail") + ', ' + getValue("logp") + '))\n');
}

function printout () {
	echo ('rk.header ("Exponential probabilities", list ("Vector of quantiles", "' + q + '", "Rate", "' + getValue ("rate") + '", "Tail", "' + getValue ("tail") + '", "Probabilities p are given as", "' + getValue ("logp") + '"))\n');
	echo ('rk.results (result, titles="Exponential probabilities")\n');
}

