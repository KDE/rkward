include ("dist_common.js");

function getDistSpecifics () {
	var rate = Number (getString ("rate"));
	return initDistSpecifics (i18n ('Exponential distribution'), 'exp', ["rate"], [0, Math.floor (5/rate)], continuous);
}
