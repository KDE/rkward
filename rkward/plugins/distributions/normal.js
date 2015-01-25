include ("dist_common.js");

function getDistSpecifics () {
	return initDistSpecifics (i18n ('Normal distribution'), 'norm', ["mean", "sd"], [-4, 4], continous);
}
