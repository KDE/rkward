include ("dist_common.js");

function getDistSpecifics () {
	var m = Number (getString ("m"));
	var n = Number (getString ("n"));
	return initDistSpecifics (i18n ('Wilcox Rank Sum distribution'), 'wilcox', ["m", "n"], [0, m*n], discrete);
}
