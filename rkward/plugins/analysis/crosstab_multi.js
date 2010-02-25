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
	echo ('rk.header ("Crosstabs (n to n)", parameters=list ("Variables"=datadescription))\n');
	echo ('\n');
	echo ('rk.print (result)\n');
}

