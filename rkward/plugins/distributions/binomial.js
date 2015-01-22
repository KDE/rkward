include ("dist_common.js");

function getDistSpecifics () {
	var dist = initDistSpecifics (i18n ('Binomial distribution'), 'binom', ["size", "prob"], [0, getString ("size")], false);
	return dist;
}
