function preprocess () {
	echo ('require (foreign)\n');
}

function calculate () {
	var options = "";

	if (getValue ("convert_dates")) {
		options += ", convert.dates=TRUE" ;
	} else {
		options += ", convert.dates=FALSE" ;
	}

	if (getValue ("convert_factors")) {
		options += ", convert.factors=TRUE" ;
	} else {
		options += ", convert.factors=FALSE" ;
	}

	if (getValue ("missing_type")) {
		options += ", missing.type=TRUE" ;
	} else {
		options += ", missing.type=FALSE" ;
	}

	if (getValue ("convert_underscore")) {
		options += ", convert.underscore=TRUE" ;
	} else {
		options += ", convert.underscore=FALSE" ;
	}

	var object = getValue ("saveto");

	echo ('data <- read.dta ("' + getValue ("file") + '"' + options + ')\n');
	echo ('\n');
	echo ('# set variable labels for use in RKWard\n');
	echo ('labels <- attr (data, "var.labels")\n');
	echo ('if (!is.null (labels)) {\n');
	echo ('        for (i in 1:length (labels)) {\n');
	echo ('                col <- make.names (attr (data, "names")[i] )\n');
	echo ('                if (!is.null (col)) {\n');
	echo ('                        rk.set.label (data[[col]], labels[i])\n');
	echo ('                }\n');
	echo ('        }\n');
	echo ('}\n');
	echo ('\n');
	echo (object + ' <<- data		# assign to globalenv()\n');
	if (getValue ("doedit") ) {
		echo ('rk.edit (' + object + ')\n');
	}
}

function printout () {
	makeHeaderCode ("Import Stata File", new Array("File", getValue ("file"), "Imported to", getValue ("saveto")));
}

