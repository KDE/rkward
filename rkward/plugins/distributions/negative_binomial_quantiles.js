// globals
var p;

function calculate () {
	p = "c (" + getString ("p").replace (/[, ]+/g, ", ") + ")";

	echo ('result <- (qnbinom (p = ' + p + ', size = ' + getValue ("size") + ', prob = ' + getValue ("prob") + ', ' + getValue ("tail") + ', ' + getValue("logp") + '))\n');
}

function printout () {
	echo ('rk.header ("Negative Binomial quantile", list ("Vector of probabilities", "' + p + '", "Size", "' + getValue ("size") + '", "Probability of success in each trial", "' + getValue ("prob") + '", "Tail", "' + getValue ("tail") + '", "Probabilities p are given as", "' + getValue ("logp") + '"))\n');
	echo ('rk.results (result, titles="Negative Binomial quantiles")\n');
}

