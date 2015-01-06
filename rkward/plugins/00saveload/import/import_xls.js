function preprocess () {
	echo ('require (gdata)\n');
}

function calculate () {
	var options = "";
	
	var header = getValue ("header");
	var verbose = getValue ("verbose");
	
	var sheet = getValue ("sheetname");
	
	var quote_char = getValue ("quote");
	if (quote_char == "other") quote_char = quote (getValue ("custom_quote"));
		
	options = ", header=" + header + ", verbose=" + verbose;

	var object = getValue ("saveto");

	echo ('data <- read.xls ("' + getValue ("file") + '", sheet="' + sheet + '"' + options + ', ');
	echo (' nrows=' + getValue ("nrows") + ', skip=' + getValue ("skip") + ', na.string="'+ getValue ("na") +'"' + getValue("strings_as_factors") + 
	      ', check.names = ' + getValue("checkname") + ', strip.white = ' + getValue("stripwhite") + ')\n');
	echo ('.GlobalEnv$' + object + ' <- data		'); comment ('assign to globalenv()');
	if (getValue ("doedit") ) {
		echo ('rk.edit (.GlobalEnv$' + object + ')\n');
	}
}

function printout () {
	new Header (i18n ("Import Microsoft EXCEL sheet"))
		.addFromUI ("file")
		.addFromUI ("saveto")
		.addFromUI ("sheetname")
		.addFromUI ("header")
		.addFromUI ("skip")
		.addFromUI ("nrows")
		.addFromUI ("na").print ();
}

