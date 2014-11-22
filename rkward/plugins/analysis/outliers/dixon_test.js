include ("outliers_common.js");

function makeTestCall () {
	echo ('		t <- dixon.test (var, type = ' + getValue ("type") + ', opposite = ' + getValue ("opposite") + ', two.sided = ' + getValue ("two_sided") + ')\n');
	echo ('		results[i, ' + i18n ("Dixon Q-statistic") + '] <- t$statistic["Q"]\n');
	echo ('		results[i, ' + i18n ("p-value") + '] <- t$p.value\n');
	echo ('		results[i, ' + i18n ("Alternative Hypothesis") + ']<- rk.describe.alternative (t)\n');
}

function printout () {
	new Header (i18n ("Dixon test for outlier")).addFromUI ("type").addFromUI ("opposite").addFromUI ("two_sided").print ();
	echo ('rk.results (results)\n');
}


