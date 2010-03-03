// globals
var q;

function calculate () {
	q = "c (" + getValue ("q").replace (/[, ]+/g, ", ") + ")";

	echo ('result <- (punif (q = ' + q + ', min = ' + getValue ("min") + ', max = ' + getValue ("max") + ', ' + getValue ("tail") + ', ' + getValue("logp") + '))\n');
}

function printout () {
	echo ('rk.header ("Uniform probability", list ("Vector of quantiles", "' + q + '", "Lower limits of the distribution", "' + getValue ("min") + '", "Upper limits of the distribution", "' + getValue ("max") + '", "Tail", "' + getValue ("tail") + '", "Probabilities p are given as", "' + getValue ("logp") + '"))\n');
	echo ('rk.results (result, titles="Uniform probabilities")\n');
}

