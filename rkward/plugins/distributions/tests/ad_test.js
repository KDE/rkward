include ("dist_test_common.js");

function testCall () {
	return ("ad.test (var)");
}

function printout (is_preview) {
	if (!is_preview) {
		echo ('rk.header (' + i18n ("Anderson-Darling Normality Test") + ')\n');
		echo ('\n');
	}
	echo ('rk.results (results)\n');
}
