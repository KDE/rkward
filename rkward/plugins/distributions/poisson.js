include ("dist_common.js");

function getDistSpecifics () {
	var lambda = Number (getString ("lambda"));
	return initDistSpecifics (i18n ('Poisson distribution'), 'pois', ["lambda"], [0, lambda + Math.sqrt (lambda)*3 + 1], discrete);
}
