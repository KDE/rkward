function preprocess () {
	// we'll need the ltm package, so in case it's not loaded...
	echo ('require(ltm)\n');
}

function calculate () {
	// let's read all values into php variables for the sake of readable code
	var spin_samples    = getValue("spin_samples");

	echo ('unidim.res <- unidimTest(' + getValue("x"));
		// check if any options must be inserted
		if (spin_samples != 100) echo(", B=" + spin_samples) ;
	echo (')\n');
}

function printout () {
	var save         = getValue("chk_save");
	var save_name    = getValue("save_name");

	echo ('rk.header ("Unidimensionality check (' + getValue("x") + ')")\n');
	echo ('rk.print (unidim.res)\n');
	// check if results are to be saved:
	if (save && save_name) {
		echo ('# keep results in current workspace\n');
		echo (save_name + ' <<- unidim.res\n');
	}
}
