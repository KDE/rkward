include ("dist_common.js");

function preprocess () {
	echo ('require (FAdist)\n');
}

function getDistSpecifics () {
	var scale = Number (getString ("scale"));
	var location = Number (getString ("location"));
	return initDistSpecifics (i18n ('Gumbel distribution'), 'gumbel', ["location", "scale"], [Math.floor (-2*scale+location), Math.ceil (5*scale+location)], continuous);
}
