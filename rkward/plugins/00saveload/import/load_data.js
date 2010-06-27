// globals
var envir;

function calculate () {
	var other_env = false;
	if (getValue ("envir.active")) {
		other_env = true;
		envir = ".GlobalEnv$" + getValue ("envir");
	} else {
		envir = "globalenv()";
	}

	if (other_env) {
		echo (envir + ' <- new.env (parent=globalenv())\n');
	}
	echo ('load (file="' + getValue("file") + '", envir=' + envir + ')\n');
}

function printout () {
	makeHeaderCode ("Load data", new Array("File",  getValue ("file"), "Import to environment",  envir));
}


