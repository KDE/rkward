include ("dist_common.js");

function getDistSpecifics () {
	var loc = Number (getString ("location"));
	var scale = Number (getString ("scale"));
	return initDistSpecifics (i18n ('Logistic distribution'), 'logis', ["location", "scale"], [-5*scale+loc, 5*scale+loc], continuous);
}
