// globals
var p;

function calculate () {
	p = "c (" + getList ("p.0").join (", ") + ")";

	echo ('result <- (qbinom (p = ' + p + ', size = ' + getValue ("size") + ', prob = ' + getValue ("prob") + ', ' + getValue ("lower") + ', ' + getValue ("logp") + '))\n');
}

function printout () {
	//produce the output

	echo ('rk.header ("Binomial quantile", list ("Vector of quantiles probabilities", "' + p + '", "Binomial trials", "' + getValue ("size") + '", "Probability of success", "' + getValue ("prob") + '", "Tail", "' + getValue ("tail") + '", "Probabilities p are given as", "' + getValue ("logp") + '"));\n');
	echo ('rk.results (result, titles="Binomial quantiles")\n');
}

