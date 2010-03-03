// globals
var q;

function calculate () {
	q = "c (" + getValue ("q").replace (/[, ]+/g, ", ") + ")";

	echo ('result <- (pbeta (q = ' + q + ', shape1 = ' + getValue ("shape1") + ', shape2 = ' + getValue ("shape2") + ', ncp = ' + getValue ("ncp") + ', ' + getValue ("tail") + ', ' + getValue("logp") + '))\n');
}

function printout () {
	echo ('rk.header ("Beta probability", list ("Vector of quantiles", "' + q + '", "Shape 1", "' + getValue ("shape1") + '", "Shape 2", "' + getValue ("shape2") + '", "non-centrality parameter (ncp)", "' + getValue ("ncp") + '", "Tail", "' + getValue ("tail") + '", "Probabilities p are given as", "' + getValue ("logp") + '"));\n');
	echo ('rk.results (result, titles="Beta probability")\n');
}

