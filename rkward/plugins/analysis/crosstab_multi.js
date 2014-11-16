function preprocess () {
	var vars = trim (getValue ("x")).replace (/\n/g, ",");

	echo ('data <- data.frame (' + vars + ', check.names=FALSE)\n');
	echo ('datadescription <- paste (rk.get.description (' + vars + '), collapse=", ");\n');
}

function calculate () {
	echo ('result <- ftable (data');
	if (!getValue ("exclude_nas.state")) echo (", exclude=NULL");
	echo (');\n');
}

function printout () {
	new Header (i18n ("Crosstabs (n to n)")).add (i18n ("Variables"), noquote (datadescription)).print ();
	echo ('\n');
	echo ('rk.print (result)\n');
}

