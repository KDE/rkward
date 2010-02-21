// globals
var envir;

function calculate () {
	var other_env = false;
	if (getValue ("other_env")) {
		other_env = true;
		envir = getValue ("envir");
	} else {
		envir = "globalenv()";
	}

	if (other_env) {
		echo ('assign ("' + envir + ', new.env (parent=globalenv()), envir=globalenv())\n');
	}
	echo ('load (file="' + getValue("file") + '", envir=' + envir + ')\n');
}

function printout () {
	makeHeaderCode ("Load data", new Array("File",  getValue ("file"), "Import to environment",  envir));
}


