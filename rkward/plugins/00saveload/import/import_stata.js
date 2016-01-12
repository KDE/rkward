include ("convert_encoding.js");

function preprocess () {
	echo ('require (foreign)\n');
	makeEncodingPreprocessCode ();
}

function preview () {
	preprocess ();
	doCalculate (true);
}

function calculate () {
	doCalculate (false);
}

function doCalculate (is_preview) {
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
	makeEncodingCall ('data');
	echo ('\n');
	comment ('set variable labels for use in RKWard');
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
	if (is_preview) {
		echo ('preview_data <- data[1:min(50,dim(data)[1]),1:min(50,dim(data)[2])]\n');
	} else {
		echo ('.GlobalEnv$' + object + ' <- data		'); comment ('assign to globalenv()');
		if (getValue ("doedit") ) {
			echo ('rk.edit (.GlobalEnv$' + object + ')\n');
		}
	}
}

function printout () {
	new Header (i18n ("Import Stata File")).addFromUI ("file").addFromUI ("saveto").print ();
}

