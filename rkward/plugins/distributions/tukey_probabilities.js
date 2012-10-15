// globals
var q;

function calculate () {
	q = "c (" + getString ("q").replace (/[, ]+/g, ", ") + ")";

	echo ('result <- (ptukey (q = ' + q + ', nmeans = ' + getValue ("nmeans") + ', df = ' + getValue ("df") + ', nranges = ' + getValue ("nranges") + ', ' + getValue ("tail") + ', ' + getValue ("logp") + '))\n');
}

function printout () {
	echo ('rk.header ("Studentized Range probability", list ("Vector of quantiles", "' + q + '", "Sample size for range", "' + getValue ("nmeans") + '", "Degrees of freedom for s", "' + getValue ("df") + '", "Number of groups whose maximum range is considered", "' + getValue ("nranges") + '", "Tail", "' + getValue ("tail") + '", "Probabilities p are given as", "' + getValue ("logp") + '"));\n');
	echo ('rk.results (result, titles="Studentized Range probabilities")\n');
}

