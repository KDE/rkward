include ("dist_common.js");

function getDistSpecifics () {
	var size = Number (getString ("size"));
	var prob = Number (getString ("prob"));
	return initDistSpecifics (i18n ('Negative Binomial distribution'), 'nbinom', ["size", "prob"], [0, Math.ceil (size * 1.5 / prob)], discrete);
}
