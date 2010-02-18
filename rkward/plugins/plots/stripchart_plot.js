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
	var plot_adds = getValue ("plotoptions.code.calculate"); //add grid and alike

	if (full) {
		echo ('rk.header ("Stripchart", list ("Variable"=rk.get.description (' + x + '), "Group"=rk.get.description (' + g + '), "Method"=' + method + params + ', "Orientation"="' + orientation + '"))\n');
		echo ('\n');
		echo ('rk.graph.on ()\n');
	}
	echo ('try (stripchart (' + x + ' ~ (' + g + '), method = ' + method + opts + getValue ("plotoptions.code.printout") + '))\n');
	if (plot_adds.length > 0) {
		echo ('\n');
		// print the grid() related code
		printIndented ("\t", plot_adds);
	}

	if (full) {
		echo ('rk.graph.off ()\n');
	}
}

