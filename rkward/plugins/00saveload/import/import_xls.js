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
	echo ('.GlobalEnv$' + object + ' <- data		# assign to globalenv()\n');
	if (getValue ("doedit") ) {
		echo ('rk.edit (.GlobalEnv$' + object + ')\n');
	}
}

function printout () {
	makeHeaderCode ("Import Microsoft EXCEL sheet", new Array("File", getValue ("file"), "Imported to", getValue ("saveto"), 
								  "Imported Sheet", getValue ("sheetname"), "First row as header", getValue ("header"), 
								  "Max number of rows to skip (-1 for no limit)", getValue ("skip"),
								  "Number of lines to read (-1 for no limit)", getValue ("nrows"),
								  "Character for missing values", getValue ("na")));
}

