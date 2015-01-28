include ("dist_common.js");

function getDistSpecifics () {
	var min = Number (getString ("min"));
	var max = Number (getString ("max"));
	return initDistSpecifics (i18n ('Uniform distribution'), 'unif', ["min", "max"], [min, max], continuous);
}
