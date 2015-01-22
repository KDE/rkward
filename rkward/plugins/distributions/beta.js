include ("dist_common.js");

function getDistSpecifics () {
	var dist = new Object;
	dist["params"] = ', shape1=' + getValue ("shape1") + ', shape2=' + getValue ("shape2") + ', ncp=' + getValue ("ncp");
	dist["funstem"] = 'beta';
	dist["header"] = new Header (i18n ('Beta distribution')).addFromUI ("shape1").addFromUI ("shape2").addFromUI ("ncp");
	return dist;
}
