include ("outliers_common.js");

function makeTestCall () {
	echo ('		t <- chisq.out.test (var, opposite = ' + getValue ("opposite") + ')\n');
	echo ('		results[i, ' + i18n ("X-squared") + '] <- t$statistic\n');
	echo ('		results[i, ' + i18n ("p-value") + '] <- t$p.value\n');
	echo ('		results[i, ' + i18n ("Alternative Hypothesis") + ']<- rk.describe.alternative (t)\n');
	echo ('		results[i, ' + i18n ("Variance") + '] <- var (var)\n');
}

function printout () {
	new Header (i18n ("Chi-squared test for outlier")).addFromUI ("opposite").print ();
	echo ('rk.results (results)\n');
}
