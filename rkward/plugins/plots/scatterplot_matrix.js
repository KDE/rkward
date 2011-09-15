function preprocess () {
	echo ('require(car)\n');
}


function printout () {
	doPrintout (true);
}

function preview () {
	preprocess ();
	doPrintout (false);
}

function doPrintout (full) {
	var vars = trim (getValue ("x")).replace (/\n/g, ",");

	echo ('data <- data.frame (' + vars + ')\n');
	echo ('\n');
	if (full) {
		echo ('rk.header ("Scatterplot Matrix", parameters=list ("Diagonal Panels", "' + getValue("diag") + '", "Plot points", "' + getValue ("plot_points") + '", "Smooth", "' + getValue ("smooth") + '", "Ellipses", "' + getValue ("ellipse") + ' at 0.5 and 0.9 levels."))\n');
		echo ('\n');
		echo ('rk.graph.on ()\n');
	}
	echo ('try (scatterplotMatrix(data, diagonal="' + getValue("diag") + '", plot.points=' + getValue ("plot_points") + ', smooth=' + getValue ("smooth") + ', ellipse=' + getValue ("ellipse") + '))\n');
	if (full) {
		echo ('rk.graph.off ()\n');
	}
}

