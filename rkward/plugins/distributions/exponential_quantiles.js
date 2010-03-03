// globals
var p;

function calculate () {
	p = "c (" + getValue ("p").replace (/[, ]+/g, ", ") + ")";

	echo ('result <- (qexp (p = ' + p + ', rate = ' + getValue ("rate") + ', ' + getValue ("tail") + ', ' + getValue("logp") + '))\n');
}

function printout () {
	echo ('rk.header ("Exponential quantiles", list ("Vector of probabilities", "' + p + '", "Rate", "' + getValue ("rate") + '", "Tail", "' + getValue ("tail") + '", "Probabilities p are given as", "' + getValue ("logp") + '"))\n');
	echo ('rk.results (result, titles="Exponential quantiles")\n');
}

