include ("dist_common.js");

function getDistSpecifics () {
	return initDistSpecifics (i18n ('Weilbull distribution'), 'weibull', ["shape", "scale"], [0, undefined], continuous);
}
