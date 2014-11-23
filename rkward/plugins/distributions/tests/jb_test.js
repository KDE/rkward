include ("dist_test_common.js");

function testCall () {
	if (getBoolean ("excludenas.state")) {
		return ("jarque.bera.test (na.omit (var))");
	}
	return ("jarque.bera.test (var)");
}

dfCall = function () {
	return ('results[i, \'df\'] <- test$parameter\n');
}

preprocess = function () {
	echo ('require (tseries)\n');	// instead of nortest
}

function printout () {
	new Header (i18n ("Jarque-Bera Normality Test")).addFromUI ("excludenas").print ();
	echo ('rk.results (results)\n');
}
