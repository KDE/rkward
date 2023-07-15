/*- This file is part of the RKWard project (https://rkward.kde.org).
SPDX-FileCopyrightText: by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
SPDX-FileContributor: The RKWard Team <rkward-devel@kde.org>
SPDX-License-Identifier: GPL-2.0-or-later
*/
// this code was generated using the rkwarddev package.
//perhaps don't make changes here, but in the rkwarddev script instead!



function preprocess(){
	// add requirements etc. here

}

function calculate(){
	// read in variables from dialog

	var x = getString("x");
	var file = getString("file");
	var quick = getString("quick");
	var dec = getString("dec");
	var customDec = getString("custom_dec");
	var sep = getString("sep");
	var customSep = getString("custom_sep");
	var rowname = getString("rowname");
	var custRowNames = getString("custRowNames");
	var colname = getString("colname");
	var custColNames = getString("custColNames");
	var encoding = getString("encoding");
	var userEncoding = getString("user_encoding");
	var qmethod = getString("qmethod");
	var na = getString("na");
	var eol = getString("eol");
	var append = getBoolean("append.state");
	var quote = getBoolean("quote.state");

	// the R code to be evaluated
	if(quick == "csv" || quick == "csv2") {
		echo("\n\t# some options can't be changed with write." + quick + "() and are set to these values:\n" + "\t# append=FALSE, sep=\"" + sep + "\", dec=\"" + dec + "\"");
	if(rowname == "TRUE") {
		echo(", col.names=NA");
	} else {
		echo(", col.names=TRUE");
	}
	echo(", qmethod=\"double\"");
	}
	if(quick != "delim" && quick != "delim2") {
		echo("\n\twrite." + quick + "(");
	} else {
		echo("\n\twrite.table(");
	}
	echo("\n\t\tx=" + x);
	echo(",\n\t\tfile=\"" + file + "\"");
	if(quick != "csv" && quick != "csv2") {
		if(append) {
			echo(",\n\t\tappend=TRUE");
		}
	}
	if(!quote) {
		echo(",\n\t\tquote=FALSE");
	}
	if(quick != "csv" && quick != "csv2" && sep != " ") {
		if(sep != "other") {
			echo(",\n\t\tsep=\"" + sep + "\"");
		} else if(customSep != " ") {
			echo(",\n\t\tsep=\"" + customSep + "\"");
		}
	}
	if(eol != "\\n") {
		echo(",\n\t\teol=\"" + eol + "\"");
	}
	if(na != "NA") {
		echo(",\n\t\tna=\"" + na + "\"");
	}
	if(quick != "csv" && quick != "csv2" && dec != ".") {
		if(dec != "other") {
			echo(",\n\t\tdec=\"" + dec + "\"");
		} else if(customDec != ".") {
			echo(",\n\t\tdec=\"" + customDec + "\"");
		}
	}
	if(rowname == "custoRow") {
		echo(",\n\t\trow.names=" + custRowNames);
	} else if(rowname != "TRUE") {
		echo(",\n\t\trow.names=" + rowname);
	}
	if(quick != "csv" && quick != "csv2") {
		if(colname == "custoCol") {
			echo(",\n\t\tcol.names=" + custColNames);
		} else if(colname != "TRUE") {
			echo(",\n\t\tcol.names=" + colname);
		}
	}
	if(quick != "csv" && quick != "csv2" && qmethod != "escape") {
		echo(",\n\t\tqmethod=\"" + qmethod + "\"");
	}
	if(encoding != "other") {
		echo(",\n\t\tfileEncoding=\"" + encoding + "\"");
	} else if(userEncoding != "") {
		echo(",\n\t\tfileEncoding=\"" + userEncoding + "\"");
	}
	echo("\n\t)\n\n");
}

function printout(){
	// printout the results


	new Header(i18n("Export Table / CSV files")).add(i18n("File"), getValue("file")).addFromUI("x").print();


}

