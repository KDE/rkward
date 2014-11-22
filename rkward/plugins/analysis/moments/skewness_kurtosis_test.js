include ("moments_common.js");
function preprocess () {} // empty for this one

function insertTestCall () {
	var narm = ", na.rm=FALSE";
	if (getValue ("narm")) narm = ", na.rm=TRUE";

	if (getValue ("skewness")) {
		echo ('		results[i, ' + i18n ("Skewness") +'] <- skewness (var' + narm + ')\n');
	}
	if (getValue ("kurtosis")) {
		echo ('		results[i, ' + i18n ("Kurtosis") + '] <- kurtosis (var' + narm + ')\n');
		echo ('		results[i, ' + i18n ("Excess Kurtosis") + '] <- results[i, \'Kurtosis\'] - 3\n');
	}
	if (getValue ("geary")) {
		echo ('		results[i, ' + i18n ("Geary Kurtosis") + '] <- geary (var' + narm + ')\n');
	}
}

function printout () {
	echo ('rk.header (' + i18n ("Skewness and Kurtosis") + ')\n');
	echo ('rk.results (results)\n');
}
