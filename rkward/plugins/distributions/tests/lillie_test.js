include ("dist_test_common.js");

function testCall () {
	return ("lillie.test (var)");
}

function printout (is_preview) {
	if (!is_preview) {
		echo ('rk.header (' + i18n ("Lilliefors (Kolmogorov-Smirnov) Normality test") + ')\n');
	}
	echo ('rk.results (results)\n');
}
