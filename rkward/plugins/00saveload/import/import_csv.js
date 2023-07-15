/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
function preview () {
	calculate (true);
}

function calculate (is_preview) {
	var tableOptions = "";
	var quick = getValue ("quick");
	if (quick == "custom") quick = "table";   // Difference only relevant in UI
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
	echo ('imported <- read.' + quick + ' (file="' + getValue("file") + '"' + tableOptions + ', '); // doing row names (what a pity...)
	var rowNameMode = getValue ("rowname");
	if (rowNameMode == "number") echo ("row.names=NULL, ");
	else if (rowNameMode == "rowcol") echo ("row.names=" + getValue("nomrow") + ", ");
	else if (rowNameMode == "custoRow") echo ("row.names=" + getValue("rownames") + ", ");

	// doing col names (what a pity...)
	if (getValue("colname") == "custoCol") echo ( "col.names = " + getValue ("colnames") + ",");
	// doing col class (what a pity...)
	if (getValue("colclass") == "custoClass") echo( "colClasses = " + getValue ("custoClasses") + ",");
	//doing what is left
	var nrows = 'nrows = ' + (is_preview ? '50' : getString ("nrows"));  // limit preview to first 50 rows for efficiency
	echo (' na.strings = "' + getValue("na") + '", ' + nrows + ', skip = ' + getValue("skip") + ', check.names = ' + getValue("checkname") + ', strip.white = ' + getValue("stripwhite") + ', blank.lines.skip = ' + getValue("blanklinesskip") + getValue("allow_escapes") + getValue("flush") + getValue("strings_as_factors") + ')\n');
	echo ('\n');
	if (is_preview) {
		echo ('preview_data <- imported[1:min(50,length(imported))]\n');  // limit preview to first 50 columns for efficiency
	} else {
		comment ('copy from the local environment to globalenv()');
		echo ('.GlobalEnv$' + getValue("name") + ' <- imported\n');
		if (getBoolean ("doedit")) {
			echo ('\n');
			echo ('rk.edit (.GlobalEnv$' + getValue ("name") + ')\n');
		}
	}
}

function printout () {
	new Header (i18n ("Import text / csv data")).addFromUI ("file").addFromUI ("name").print ();
}
