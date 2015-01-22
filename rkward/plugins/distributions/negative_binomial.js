include ("dist_common.js");

function getDistSpecifics () {
	return initDistSpecifics (i18n ('Negative Binomial distribution'), 'nbinom', ["size", "prob"]);
}
