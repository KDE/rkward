include ("dist_common.js");

function getDistSpecifics () {
	var prob = Number (getString ("prob"));
	return initDistSpecifics (i18n ('Geometric distribution'), 'geom', ["prob"], [0, Math.floor (5/prob)], discrete);
}
