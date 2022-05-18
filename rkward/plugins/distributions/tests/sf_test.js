include ("dist_test_common.js");

function testCall () {
	return ("sf.test (var)");
}

function printout (is_preview) {
	if (!is_preview) {
		echo ('rk.header (' + i18n ("Shapiro-Francia Normality Test") + ')\n');
	}
	echo ('rk.results (results)\n');
}
