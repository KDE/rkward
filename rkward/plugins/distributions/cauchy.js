include ("dist_common.js");

function getDistSpecifics () {
	return initDistSpecifics (i18n ('Cauchy distribution'), 'cauchy', ["location", "scale"], [], continuous);
}
