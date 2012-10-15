// globals
var q;

function calculate () {
	q = "c (" + getString ("q").replace (/[, ]+/g, ", ") + ")";

	echo ('result <- (pnbinom (q = ' + q + ', size = ' + getValue ("size") + ', prob = ' + getValue ("prob") + ', ' + getValue ("tail") + ', ' + getValue("logp") + '))\n');
}

function printout () {
	echo ('rk.header ("Negative Binomial probability", list ("Vector of quantiles", "' + q + '", "Size", "' + getValue ("size") + '", "Probability of success in each trial", "' + getValue ("prob") + '", "Tail", "' + getValue ("tail") + '", "Probabilities p are given as", "' + getValue ("logp") + '"))\n');
	echo ('rk.results (result, titles="Negative Binomial probabilities")\n');
}

