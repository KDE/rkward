function printout () {
	var x = getValue ("x");
	var scale = getValue ("scale");
	var width = getValue ("width");
	var atom = getValue ("atom");

	echo ('rk.header ("Stem-and-Leaf Plot",\n');
	echo ('	parameters=list ("Variable", paste (rk.get.description (' + x + ')), "Plot Length", "' + scale + '","Plot Width", "' + width + '", "Tolerance", "' + atom + '"))\n');
	echo ('\n');
	echo ('rk.print.literal(capture.output(stem(' + x + ', scale = ' + scale + ', width = ' + width + ', atom = ' + atom + ')))\n');
}

