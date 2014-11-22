include ("moments_common.js");

function insertTestCall () {
	echo ('		# This is the core of the calculation\n');
	echo ('		t <- agostino.test (var, alternative = "' + getValue ("alternative") + '")\n');
	echo ('		results[i, \'skewness estimator (skew)\'] <- t$statistic["skew"]\n');
	echo ('		results[i, \'transformation (z)\'] <- t$statistic["z"]\n');
	echo ('		results[i, \'p-value\'] <- t$p.value\n');
	if (getValue ("show_alternative")) {
		echo ('		results[i, \'Alternative Hypothesis\'] <- rk.describe.alternative (t)\n');
	}
}

function printout () {
	new Header (i18n ("D'Agostino test of skewness")).addFromUI ("alternative").print ();
	echo ('rk.results (results)\n');
}


