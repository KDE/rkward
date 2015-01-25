include ("dist_common.js");

function getDistSpecifics () {
	return initDistSpecifics (i18n ('Beta distribution'), 'beta', ["shape1", "shape2", "ncp"], [0, 1], continuous);
}
