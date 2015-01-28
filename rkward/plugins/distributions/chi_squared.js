include ("dist_common.js");

function getDistSpecifics () {
	var df = Number (getString ("df"));
	return initDistSpecifics (i18n ('Chi-squared distribution'), 'chisq', ["df", "ncp"],
	                          [Math.max (0, Math.floor (df * .6 - 4)), Math.floor (df * 1.4 + 4)], // NOTE: crude self-made heuristic for covering the likely range of interest for auto-quantiles
	                          continuous);
}
