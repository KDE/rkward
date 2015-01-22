include ("dist_common.js");

function getDistSpecifics () {
	var dist = new Object;
	var size = getString ("size");
	dist["params"] = ', size=' + size + ", prob=" + getString ("prob");
	dist["funstem"] = 'binom';
	dist["max_quantile"] = size;
	dist["header"] = new Header (i18n ('Binomial distribution')).addFromUI ("size").addFromUI ("prob");
	return dist;
}
