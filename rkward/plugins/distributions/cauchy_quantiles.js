// globals
var p;

function calculate () {
	p = "c (" + getValue ("p").replace (/[, ]+/g, ", ") + ")";

	echo ('result <- (qcauchy (p = ' + p + ', location = ' + getValue ("location") + ', scale = ' + getValue ("scale") + ', ' + getValue ("tail") + ', ' + getValue("logp") + '))\n');
}

function printout () {
	echo ('rk.header ("Cauchy quantiles", list ("Vector of probabilities", "' + p + '", "Location", "' + getValue ("location") + '", "Scale", "' + getValue ("scale") + '", "Tail", "' + getValue ("tail") + '", "Probabilities p are given as", "' + getValue ("logp") + '"));\n');
	echo ('rk.results (result, titles="Cauchy quantiles")\n');
}

