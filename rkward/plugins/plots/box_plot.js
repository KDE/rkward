function preview () {
	doPrintout (false);
}

function printout () {
	doPrintout (true);
}

function doPrintout (full) {
	var xvarsstring = "";
	var names_mode = "";
	var mean = "";
	var pch_mean = "";
	var sd = "";
	var pch_sd_high = "";
	var pch_sd_low = "";
	var horizontal = "";
	var plot_adds = "";
	xvarsstring = getValue ("x").split ("\n").join (", ");
	names_mode = getValue ("names_mode");
	mean = getValue ("mean");
	pch_mean = getValue ("pch_mean");
	sd = getValue ("sd");
	pch_sd_high = getValue ("pch_sd_high");
	pch_sd_low = getValue ("pch_sd_low");
	horizontal = getValue ("orientation");
	plot_adds = getValue ("plotoptions.code.calculate"); //add grid and alike


	echo ('data_list <- list (' + xvarsstring + ')		#convert single sample variables to list\n');
	if (names_mode == "rexp") {
		echo ("names(data_list) <- " + getValue ("names_exp") + "\n");
	} else if (names_mode == "custom") {
		echo ("names(data_list) <- c (\"" + str_replace (";", "\", \"", trim (getValue ("names_custom"))) + "\")\n");
	}

	if (full) {
		echo ('rk.header ("Boxplot", list ("Variable(s)", rk.get.description (' + xvarsstring + ', paste.sep=", ")))\n');
		echo ('rk.graph.on()\n');
	}
	echo ('try (boxplot (data_list, notch = ' + getValue ("notch") + ', outline = ' + getValue("outline") + ', horizontal = ' + getValue("orientation") + getValue ("plotoptions.code.printout") + ')) #actuall boxplot function\n');
	if ((mean == "TRUE") && (horizontal == "TRUE")) {
		echo ('	try (points(1:length(data_list) ~ sapply(data_list,mean,na.rm = TRUE),pch=' + pch_mean + ', cex = ' + getValue ("cex_sd_mean") + getValue ("sd_mean_color.code.printout") + ')) #calculates the mean for all data and adds a point at the corresponding position\n');
	}
	if ((mean == "TRUE") && (horizontal == "FALSE")) {
		echo ('	try (points(sapply(data_list,mean,na.rm = TRUE),pch=' + pch_mean + ', cex = ' + getValue ("cex_sd_mean") + getValue ("sd_mean_color.code.printout") + ')) #calculates the mean for all data and adds a point at the corresponding position\n');
	}

	if ((sd == "TRUE") && (horizontal == "FALSE")) {
		echo ('	sd_low <- (sapply(data_list,mean,na.rm = TRUE)) - (sapply(data_list,sd,na.rm = TRUE))\n');
		echo ('	sd_high <- (sapply(data_list,mean,na.rm = TRUE)) + (sapply(data_list,sd,na.rm = TRUE))\n');
		echo ('	points(sd_low,pch=' + pch_sd_low + ', cex = ' + getValue ("cex_sd_mean") + getValue ("sd_mean_color.code.printout") + ')\n');
		echo ('	points(sd_high,pch=' + pch_sd_high + ', cex = ' + getValue ("cex_sd_mean") + getValue ("sd_mean_color.code.printout") + ')\n');
	}
	if ((sd == "TRUE") && (horizontal == "TRUE")) {
		echo ('	sd_low <- (sapply(data_list,mean,na.rm = TRUE)) - (sapply(data_list,sd,na.rm = TRUE))\n');
		echo ('	sd_high <- (sapply(data_list,mean,na.rm = TRUE)) + (sapply(data_list,sd,na.rm = TRUE))\n');
		echo ('	points(1:length(data_list) ~ sd_low,pch=' + pch_sd_low + ', cex = ' + getValue ("cex_sd_mean") + getValue ("sd_mean_color.code.printout") + ')\n');
		echo ('	points(1:length(data_list) ~ sd_high,pch=' + pch_sd_high + ', cex = ' + getValue ("cex_sd_mean") + getValue ("sd_mean_color.code.printout") + ')\n');
	}

	if (plot_adds.length > 0) {
		echo ('\n');
		// print the grid() related code
		printIndented ("\t", plot_adds);
	}

	if (full) {
		echo ('rk.graph.off ()\n');
	}
}

