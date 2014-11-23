include ("dist_test_common.js");

preprocess = function () {}

function testCall () {
	return ("shapiro.test (var)");
}

function printout () {
	echo ('rk.header (' + i18n ("Shapiro-Wilk Normality Test") + ')\n');
	echo ('rk.results (results)\n');
}
