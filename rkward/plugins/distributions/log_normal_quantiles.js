// globals
var p;

function calculate () {
	p = "c (" + getValue ("p").replace (/[, ]+/g, ", ") + ")";

	echo ('result <- (qlnorm (p = ' + p + ', meanlog = ' + getValue ("meanlog") + ', sdlog = ' + getValue ("sdlog") + ', ' + getValue ("tail") + ', ' + getValue ("logp") + '))\n');
}

function printout () {
	echo ('rk.header ("Log Normal quantile", list ("Vector of probabilities", "' + p + '", "meanlog", "' + getValue ("meanlog") + '", "sdlog", "' + getValue ("sdlog") + '", "Tail", "' + getValue ("tail") + '", "Probabilities p are given as", "' + getValue ("logp") + '"))\n');
	echo ('rk.results (result, titles="Log Normal quantiles")\n');
}

