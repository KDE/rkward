include ("dist_common.js");

function getDistSpecifics () {
	return initDistSpecifics (i18n ('Hypergeometric distribution'), 'hyper', ["m", "n", "k"], [0, getString ("m")], discrete);
}
