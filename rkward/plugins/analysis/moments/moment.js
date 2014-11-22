include ("moments_common.js");

function insertTestCall () {
	echo ('		results[i, ' + i18nc ("Statistical moment", "Moment") + '] <- moment (var, order = ' + getValue ("order") + ', central = ' + getValue ("central") + ', absolute = ' + getValue ("absolute") + ', na.rm = ' + getValue ("narm") + ')\n');
}

function printout () {
	new Header (i18n ("Statistical Moment")).addFromUI ("order").addFromUI ("central").addFromUI ("absolute").addFromUI ("narm").print ();
	echo ('rk.results (results)\n');
}

