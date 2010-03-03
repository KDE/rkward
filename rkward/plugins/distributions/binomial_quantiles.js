// globals
var p;

function calculate () {
	p = "c (" + getValue ("p").replace (/[, ]+/g, ", ") + ")";

	echo ('result <- (qbinom (p = ' + p + ', size = ' + getValue ("size") + ', prob = ' + getValue ("prob") + ', ' + getValue ("tail") + ', ' + getValue ("logp") + '))\n');
}

function printout () {
	//produce the output

	echo ('rk.header ("Binomial quantile", list ("Vector of quantiles probabilities", "' + p + '", "Binomial trials", "' + getValue ("size") + '", "Probability of success", "' + getValue ("prob") + '", "Tail", "' + getValue ("tail") + '", "Probabilities p are given as", "' + getValue ("logp") + '"));\n');
	echo ('rk.results (result, titles="Binomial quantiles")\n');
}

