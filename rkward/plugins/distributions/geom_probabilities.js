// globals
var q;

function calculate () {
	q = "c (" + getValue ("q").replace (/[, ]+/g, ", ") + ")";

	echo ('result <- (pgeom (q = ' + q + ', prob = ' + getValue ("prob") + ', ' + getValue ("tail") + ', ' + getValue("logp") + '))\n');
}

function printout () {
	echo ('rk.header ("Geometric probability", list ("Vector of quantiles", "' + q + '", "Probability of success in each trial", "' + getValue ("prob") + '", "Tail", "' + getValue ("tail") + '", "Probabilities p are given as", "' + getValue ("logp") + '"))\n');
	echo ('rk.results (result, titles="Geometric probabilities")\n');
}

