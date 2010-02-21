function calculate () {
	var vars = trim (getValue ("data")).replace (/\n/g, "','");

	echo ('package.skeleton(name="' + getValue("name") + '", list=c(\'' + vars + '\'), path="' + getValue("path") + '", force= ' + getValue("force") + ')\n');
}

function printout () {
	makeHeaderCode ("Create package skeleton", new Array("Name", getValue ("name"), "Directory", getValue ("path")));
}

