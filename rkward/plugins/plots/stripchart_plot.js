function printout () {
	doPrintout (true);
}

function preview () {
	doPrintout (false);
}

function doPrintout (full) {
	var opts = "";
	var params = "";
	var x = getValue ("x");
	var g = getValue ("g");
	var method = '"' + getValue ("method") + '"';
	if (method == "\"jitter\"") {
		opts += ", jitter = " + getValue ("jitter");
		params += ", \"Jitter\" = " + getValue ("jitter");
	} else if (method == "\"stack\"") {
		opts += ", offset = " + getValue ("offset");
		params += ", \"Offset\" = " + getValue ("offset");
	}
	var orientation = getValue ("orientation");
	if (orientation == "Vertical") opts += ", vertical = TRUE";

	if (full) {
		echo ('rk.header ("Stripchart", list ("Variable"=rk.get.description (' + x + '), "Group"=rk.get.description (' + g + '), "Method"=' + method + params + ', "Orientation"="' + orientation + '"))\n');
		echo ('\n');
		echo ('rk.graph.on ()\n');
	}
	echo ('try ({\n');
	printIndentedUnlessEmpty ("\t", getValue ("plotoptions.code.preprocess"), '', '\n');

	echo ('\tstripchart (' + x + ' ~ (' + g + '), method = ' + method + opts + getValue ("plotoptions.code.printout") + ')\n');

	printIndentedUnlessEmpty ("\t", getValue ("plotoptions.code.calculate"), '\n', '');
	echo ('})\n');

	if (full) {
		echo ('rk.graph.off ()\n');
	}
}

