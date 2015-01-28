include ("dist_common.js");

function getDistSpecifics () {
	return initDistSpecifics (i18n ('Gamma distribution'), 'gamma', ["shape", "rate"], [0, undefined], continuous);
}
