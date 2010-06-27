function preprocess () {
	echo ('require (foreign)\n');
	if (getValue ("do_locale_conversion")) {
		echo ('\n');
		echo ('# helper function to convert all strings to the current encoding\n');
		echo ('iconv.recursive <- function (x, from) {\n');
		echo ('	attribs <- attributes (x);\n');
		echo ('	if (is.character (x)) {\n');
		echo ('		x <- iconv (x, from=from, to="", sub="")\n');
		echo ('	} else if (is.list (x)) {\n');
		echo ('		x <- lapply (x, function (sub) iconv.recursive (sub, from))\n');
		echo ('	}\n');
		echo ('	# convert factor levels and all other attributes\n');
		echo ('	attributes (x) <- lapply (attribs, function (sub) iconv.recursive (sub, from))\n');
		echo ('	x\n');
		echo ('}\n');
	}
}

function calculate () {
	var data_frame = "";
	var data_frame_opt = "";
	if (getValue ("data_frame")) {
		data_frame = true;
		data_frame_opt = ", to.data.frame=TRUE";
	}

	var labels_opt = "";
	if (getValue ("use_labels")) {
		labels_opt += ", max.value.labels=" + getValue ("labels_limit");
		if (getValue ("trim_labels")) labels_opt += ", trim.factor.names=TRUE";
	} else {
		labels_opt = ", use.value.labels=FALSE";
	}

	var object = getValue ("saveto");

	echo ('data <- read.spss ("' + getValue ("file") + '"' + data_frame_opt + labels_opt + ')\n');
	if (getValue ("do_locale_conversion")) {
		var from_locale = getValue ("encoding");
		if (from_locale == "other") {
			from_locale = getValue ("user_encoding");
		}
		echo ('\n');
		echo ('# convert all strings to the current encoding\n');
		echo ('data <- iconv.recursive (data, from="' + from_locale + '")\n');
	}
	if (getValue ("convert_var_labels")) {
		echo ('\n');
		echo ('# set variable labels for use in RKWard\n');
		echo ('labels <- attr (data, "variable.labels");\n');
		echo ('if (!is.null (labels)) {\n');
		echo ('	for (i in 1:length (labels)) {\n');
		echo ('		col <- make.names (names (labels[i]))\n');
		echo ('		if (!is.null (col)) {\n');
		echo ('			rk.set.label (data[[col]], labels[i])\n');
		echo ('		}\n');
		echo ('	}\n');
		echo ('}\n');
	}
	echo ('\n');
	echo ('.GlobalEnv$' + object + ' <- data		# assign to globalenv()\n');
	if (getValue ("doedit") && data_frame) {
		echo ('rk.edit (.GlobalEnv$' + object + ')\n');
	}
}

function printout () {
	makeHeaderCode ("Import SPSS data", new Array("File",  getValue ("file"), "Import as", getValue ("saveto")));
}

