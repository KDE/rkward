// globals
var p;

function calculate () {
	p = "c (" + getString ("p").replace (/[, ]+/g, ", ") + ")";

	echo ('result <- (qtukey (p = ' + p + ', nmeans = ' + getValue ("nmeans") + ', df = ' + getValue ("df") + ', nranges = ' + getValue ("nranges") + ', ' + getValue ("tail") + ', ' + getValue ("logp") + '))\n');
}

function printout () {
	echo ('rk.header ("Studentized Range quantiles", list ("Vector of probabilities", "' + p + '", "Sample size for range", "' + getValue ("nmeans") + '", "Degrees of freedom for s", "' + getValue ("df") + '", "Number of groups whose maximum range is considered", "' + getValue ("nranges") + '", "Tail", "' + getValue ("tail") + '", "Probabilities p are given as", "' + getValue ("logp") + '"));\n');
	echo ('rk.results (result, titles="Studentized Range quantiles")\n');
}

