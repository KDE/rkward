function calculate () {
	var prompt = "";
	// most options should only be shown, if they differ from the default.
	var options = "";
	if (getValue ("echo")) {
		options += ", echo=TRUE";
		var prompt = getValue ("promptecho");
		if (prompt.length > 0) {
			options += ", prompt.echo=\"" + prompt + "\"";
		}
		options += ", max.deparse.length=" + getValue ("maxdeparselength");
		options += ", verbose=" + getValue ("verbose");
	} else {
		options += ", verbose=FALSE";
	}
	options += ", print.eval=" + getValue ("printeval");

	echo ('source (file="' + getValue("file") + '", local=' + getValue("local") + options + ', chdir=' + getValue("chdir") + ')\n');
}

function printout () {
	makeHeaderCode ("Source R file", new Array("File", getValue ("file")));
}


