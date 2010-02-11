var dodensity_plot;

function preprocess () {
	dodensity_plot = (getValue ("plot_type") == "density_plot");

	if (!dodensity_plot) {
		echo ('require(hdrcde)\n');
	}
}


function preview () {
	preprocess ();
	doPrintout (false);
}

function printout () {
	doPrintout (true);
}

function doPrintout (full) {
	var adjust = getValue ("adjust");
	var x = getValue ("x");
	var resolution = getValue ("n");
	var narm = getValue ("narm");
	var kern = getValue ("kern");
	
	var bw = "";
	if (kern == "gaussian") {
		bw = getValue ("bw");
	}
	var dorug = getValue ("rug");

	var density_call = "density(" + x;
	if (bw != "") density_call += ", bw=\"" + bw + "\"";
	density_call += ", adjust=" + adjust + ", kern=\"" + kern + "\", n=" + resolution + ", " + narm + ")";

	var title = "";
	if (dodensity_plot) title = "Density Plot";
	else title = "Highest density regions";

	if (full) {
		echo ('rk.header ("' + title + '", list ("Variable", rk.get.description (' + x + ')');
		if (bw != "") {
			echo (', "Band Width", "');
			echo (bw);
			echo ('"');
		}
		echo (', "Adjust", ' + adjust + ', "Remove Missing Values", ' + narm + ', "Length", length (' + x + '), "Resolution", ' + resolution + ', "Smoothing Kernel", "' + kern + '"))\n');
		echo ('\n');
		echo ('rk.graph.on ()\n');
	}

	echo ('try ({\n');
	if (dodensity_plot) {
		echo ('	plot(' + density_call + getValue ("plotoptions.code.printout") + ')\n');
	} else {
		echo ('	hdr.den(den=' + density_call + getValue ("plotoptions.code.printout") + ')\n');
	}
	if (dorug) {
		echo ('	rug(' + x + ', ' + getValue ("rug_ticksize") + ', ' + getValue ("rug_lwd") + ', ' + getValue ("rug_side") + getValue ("rug_col.code.printout") + ')\n');
	}
	echo ('})\n');
	if (full) {
		echo ('rk.graph.off ()\n');
	}
}

