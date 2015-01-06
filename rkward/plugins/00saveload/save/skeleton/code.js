function calculate () {
	var vars = trim (getValue ("data")).replace (/\n/g, "','");

	echo ('package.skeleton(name="' + getValue("name") + '", list=c(\'' + vars + '\'), path="' + getValue("path") + '", force= ' + getValue("force") + ')\n');
}

function printout () {
	new Header (i18n ("Create package skeleton")).addFromUI ("name").addFromUI ("path").print ();
}

