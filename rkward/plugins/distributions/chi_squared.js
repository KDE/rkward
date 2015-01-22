include ("dist_common.js");

function getDistSpecifics () {
	return initDistSpecifics (i18n ('Chi-squared distribution'), 'chisq', ["df", "ncp"]);
}
