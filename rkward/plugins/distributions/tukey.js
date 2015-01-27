include ("dist_common.js");

function getDistSpecifics () {
	return initDistSpecifics (i18n ('Studentized Range (Tukey) distribution'), 'tukey', ["nmeans", "df"], [0, undefined], continuous);
}
