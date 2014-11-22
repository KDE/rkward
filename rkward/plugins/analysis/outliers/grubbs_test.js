include ("outliers_common.js");

function makeTestCall () {
	echo ('		t <- grubbs.test (var, type = ' + getValue ("type") + ', opposite = ' + getValue ("opposite") + ', two.sided = ' + getValue ("two_sided") + ')\n');
	echo ('		results[i, \'G\'] <- t$statistic["G"]\n');
	echo ('		results[i, \'U\'] <- t$statistic["U"]\n');
	echo ('		results[i, ' + i18n ("p-value") + '] <- t$p.value\n');
	echo ('		results[i, ' + i18n ("Alternative Hypothesis") + ']<- rk.describe.alternative (t)\n');
}

function printout () {
	new Header (i18n ("Grubbs tests for one or two outliers in data sample"))
	    .addFromUI ("type").addFromUI ("opposite").addFromUI ("two_sided").print ();
	echo ('rk.results (results)\n');
}


