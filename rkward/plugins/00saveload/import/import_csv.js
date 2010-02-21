function calculate () {
	var tableOptions = "";
	var quick = getValue ("quick");
	if (quick == "table") {
		var dec = getValue ("dec");
		if (dec == "other") dec = quote (getValue ("custom_dec"));
		var sep = getValue ("sep");
		if (sep == "other") sep = quote (getValue ("custom_sep"));
		var quote_char = getValue ("quote");
		if (quote_char == "other") quote_char = quote (getValue ("custom_quote"));
		var header = getValue ("header");
		var fill = getValue ("fill");
		var comchar = quote (getValue ("commentchar"));
		tableOptions = ", header=" + header + ", sep=" + sep + ", quote=" + quote_char + ", dec=" + dec + ", fill=" + fill + ", comment.char=" + comchar;
	} else {
		tableOptions = "";
	}
// Other method is to use read.table and show all the options - more transparent
	echo ('imported <<- read.' + quick + ' (file="' + getValue("file") + '"' + tableOptions + ', '); // doing row names (what a pity...)
	if (getValue("rowname")!="NULL") {
		echo ("row.names = ");
		if (getValue("rowname")=="rowcol") echo (getValue("nomrow") + ",");
		else echo (getValue("rownames") + ",");
	}
	// doing col names (what a pity...)
	if (getValue("colname") == "custoCol") echo ( "col.names = " + getValue ("colnames") + ",");
	// doing col class (what a pity...)
	if (getValue("colclass") == "custoClass") echo( "colClasses = " + getValue ("custoClasses") + ",");
	//doing what is left
	echo (' na.strings = "' + getValue("na") + '", nrows = ' + getValue("nrows") + ', skip = ' + getValue("skip") + ', check.names = ' + getValue("checkname") + ', strip.white = ' + getValue("stripwhite") + ', blank.lines.skip = ' + getValue("blanklinesskip") + getValue("allow_escapes") + getValue("flush") + getValue("strings_as_factors") + ')\n');
	echo ('\n');
	echo ('# copy from the local environment to globalenv()\n');
	echo ('assign("' + getValue("name") + '", imported, envir=globalenv())\n');
	if (getValue ("doedit")) {
		echo ('\n');
		echo ('rk.edit (' + getValue ("name") + ')\n');
	}
}

function printout () {
	makeHeaderCode ("Import text / csv data", new Array("File" ,  getValue ("file"), "Import as" ,  getValue ("name")));
}

