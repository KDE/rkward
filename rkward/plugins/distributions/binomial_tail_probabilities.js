// globals
var q;

function calculate () {
	q = "c (" + getValue ("q").replace (/[, ]+/g, ", ") + ")";

	echo ('result <- (pbinom (q = ' + q + ', size = ' + getValue ("size") + ', prob = ' + getValue ("prob") + ', ' + getValue ("tail") + ', ' + getValue ("logp") + '))\n');
}

function printout () {
	echo ('rk.header ("Binomial tail probability", list ("Vector of quantiles", "' + q + '", "Binomial trials", "' + getValue ("size") + '", "Probability of success", "' + getValue ("prob") + '", "Tail", "' + getValue ("tail") + '", "Probabilities p are given as", "' + getValue ("logp") + '"));\n');
	echo ('rk.results (result, titles="Binomial tail probabilities")\n');
}

