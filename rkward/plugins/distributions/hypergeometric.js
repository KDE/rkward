include ("dist_common.js");

function getDistSpecifics () {
	var dist = initDistSpecifics (i18n ('Hypergeometric distribution'), 'hyper', ["m", "n", "k"]);
	dist["max_quantile"] = getString ("m");
	return dist;
}
