include ("outliers_common.js");

function makeTestCall () {
	echo ('		t <- outlier (var, opposite = ' + getValue ("opposite") + ')\n');
	echo ('		results[i, \'Outlier\'] <- t\n');
}

function printout (is_preview) {
	if (!is_preview) {
		new Header (i18n ("Find potential outlier")).addFromUI ("opposite").print ();
	}
	echo ('rk.results (results)\n');
}


