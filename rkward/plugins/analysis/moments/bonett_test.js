include ("moments_common.js");

function insertTestCall () {
	echo ('		t <- bonett.test (var, alternative = "' + getValue ("alternative") + '")\n');
	echo ('		results[i, \'Kurtosis estimator (tau)\'] <- t$statistic["tau"]\n');
	echo ('		results[i, \'Transformation (z)\'] <- t$statistic["z"]\n');
	echo ('		results[i, \'p-value\'] <- t$p.value\n');
	if (getValue ("show_alternative")) {
		echo ('		results[i, \'Alternative Hypothesis\'] <- rk.describe.alternative (t)\n');
	}
}

function printout () {
	new Header (i18n ("Bonett-Seier test of Geary's kurtosis")).addFromUI ("alternative").print ();
	echo ('rk.results (results)\n');
}
