include ("dist_common.js");

function getDistSpecifics () {
	return initDistSpecifics (i18n ('Weibull distribution'), 'weibull', ["shape", "scale"], [0, undefined], continuous);
}
