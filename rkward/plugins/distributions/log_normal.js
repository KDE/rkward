include ("dist_common.js");

function getDistSpecifics () {
	return initDistSpecifics (i18n ('Log Normal distribution'), 'lnorm', ["meanlog", "sdlog"]);
}
