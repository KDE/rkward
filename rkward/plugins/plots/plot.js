function printout () {
	doPrintout (true);
}

function preview () {
	doPrintout (false);
}

function doPrintout (full) {
	var x = getValue ("xvarslot");
	var y = getValue ("yvarslot");
	if (!(y == "")) {
		y = ", " + y;
	}

	if (full) {

		echo ('rk.header ("Generic Plot")\n');
		echo ('rk.graph.on ()\n');
	}

	echo ('try({\n');
	printIndentedUnlessEmpty ("\t", getValue ("plotoptions.code.preprocess"), '', '\n');

	echo ('	plot(' + x + y + getValue ("plotoptions.code.printout") + ');\n');

	printIndentedUnlessEmpty ("\t", getValue ("plotoptions.code.calculate"), '\n', '');
	echo ('})\n');
	if (full) {

		echo ('rk.graph.off ()\n');
	}
}

