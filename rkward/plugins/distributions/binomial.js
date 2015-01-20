include ("dist_common.js");

function getDistSpecifics () {
	var dist = new Object;
	dist["params"] = "bla";
	dist["funstem"] = "binom";
	dist["header"] = new Header (i18n ("Binomial tail probability"));
	return dist;
}

/*function calculate () {
	q = "c (" + getString ("q").replace (/[, ]+/g, ", ") + ")";

	echo ('result <- (pbinom (q = ' + q + ', size = ' + getValue ("size") + ', prob = ' + getValue ("prob") + ', ' + getValue ("tail") + ', ' + getValue ("logp") + '))\n');
}

function printout () {
	echo ('rk.header (, list ("Vector of quantiles", "' + q + '", "Binomial trials", "' + getValue ("size") + '", "Probability of success", "' + getValue ("prob") + '", "Tail", "' + getValue ("tail") + '", "Probabilities p are given as", "' + getValue ("logp") + '"));\n');
	echo ('rk.results (result, titles="Binomial tail probabilities")\n');
}

*/