include ("dist_test_common.js");

function testCall () {
	return ("cvm.test (var)");
}

function printout (is_preview) {
	if (!is_preview) {
		echo ('rk.header (' + i18n ("Cramer-von Mises Normality Test") + ')\n');
	}
	echo ('rk.results (results)\n');
}
