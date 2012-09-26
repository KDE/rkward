var x;
var y;

function calculate () {
	x = str_replace ("\n", ",", trim (getValue ("x"))) ;
	y = str_replace ("\n", ",", trim (getValue ("y"))) ;

	var type = "";
	if (getValue ("manual_type") == "true") {
		type = getValue ("custom_type");
	} else {
		type = "c ('" + getValue ("pointtype") + "')";
	}
	var col = getValue ("col");
	var pch = getValue ("pch");
	var cex = getValue ("cex");

// input
	echo ('Xvars <- list(' + x + ')\n');
	echo ('Yvars <- list(' + y + ')\n');
	echo ('\n');

// verification (is this needed?) ?>
	echo ('if (length(Xvars) != length(Yvars)) {\n');
	echo ('	stop("Unequal number of X and Y variables given")\n');
	echo ('}\n');

	echo ('# find range of X/Y values needed\n');
	echo ('Xrange <- range (c (Xvars), na.rm=TRUE)\n');
	echo ('Yrange <- range (c (Yvars), na.rm=TRUE)\n');
	echo ('\n');
	echo ('type <- rep (' + type + ', length.out=length (Xvars));\n');
	echo ('col <- rep (' + col + ', length.out=length (Xvars));\n');
	echo ('cex <- rep (' + cex + ', length.out=length (Xvars));\n');
	echo ('pch <- rep (' + pch + ', length.out=length (Xvars));\n');
}

function printout () {
	doPrintout (true);
}

function preview () {
	calculate ();
	doPrintout (false);
}

function doPrintout (full) {
	if (full) {
		echo ('rk.header ("Scatterplot", parameters = list (\n');
		echo ('	"X variables"=paste (rk.get.description (' + x + '), collapse=", "),\n');
		echo ('	"Y variables"=paste (rk.get.description (' + y + '), collapse=", ")))\n');
		echo ('\n');
		echo ('rk.graph.on()\n');
		echo ('\n');
	}
	echo ('try ({\n');
	printIndentedUnlessEmpty ("\t", getValue ("plotoptions.code.preprocess"), '', '\n');

	echo ('	# make frame and axes\n');
	echo ('	plot(Xrange, Yrange, type="n"' + getValue ("plotoptions.code.printout") + ')\n');
	echo ('	\n');
	echo ('	# plot variables one X/Y pair at a time\n');
	echo ('	for (i in 1:length(Xvars)) {\n');
	echo ('		points (\n');
	echo ('			Xvars[[i]],\n');
	echo ('			Yvars[[i]],\n');
	echo ('			type = type[[i]],\n');
	echo ('			col = col[[i]],\n');
	echo ('			cex = cex[[i]],\n');
	echo ('			pch = pch[[i]]\n');
	echo ('		)\n');
	echo ('	}\n');

	printIndentedUnlessEmpty ("\t", getValue ("plotoptions.code.calculate"), '\n', '');
	echo ('})\n');
	if (full) {
		echo ('\n');
		echo ('rk.graph.off()\n');
	}
}

