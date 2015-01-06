function calculate () {
	var sep = getValue ("sep");
	if (sep == "other") sep = getValue ("custom_sep");
	if (sep == " ") sep = "";
	else sep = ", sep=" + quote (sep);

	echo ('write (x=' + getValue("data") + ', file="' + getValue("file") + '", ncolumns=' + getValue("ncolumns") + ', append=' + getValue("append") + sep + ')\n');
}

function printout () {
	new Header (i18n ("Write Variables")).addFromUI ("file").addFromUI ("data").print ();
}

