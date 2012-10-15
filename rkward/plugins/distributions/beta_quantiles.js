// globals
var p;

function calculate () {
	p = "c (" + getString ("p").replace (/[, ]+/g, ", ") + ")";

	echo ('result <- (qbeta (p = ' + p + ', shape1 = ' + getValue ("shape1") + ', shape2 = ' + getValue ("shape2") + ', ncp = ' + getValue ("ncp") + ', ' + getValue ("tail") + ', ' + getValue("logp") + '))\n');
}

function printout () {
	echo ('rk.header ("Beta quantiles", list ("Vector of probabilities", "' + p + '", "Shape 1", "' + getValue ("shape1") + '", "Shape 2", "' + getValue ("shape2") + '", "non-centrality parameter (ncp)", "' + getValue ("ncp") + '", "Tail", "' + getValue ("tail") + '", "Probabilities p are given as", "' + getValue ("logp") + '"));\n');
	echo ('rk.results (result, titles="Beta quantiles")\n');
}

