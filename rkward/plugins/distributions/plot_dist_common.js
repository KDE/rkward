// globals
var options;

function printout () {
	doPrintout (true);
}

function preview () {
	do_preprocess ();	// do_preprocess calls preprocess(), if, and only if that is defined
	do_calculate ();
	doPrintout (false);
}

// get the range parameters for the continuous distributions (it's all the same for these)
function getContRangeParameters () {
	options['n'] = getValue ("n");
	options['min'] = getValue ("min");
	options['max'] = getValue ("max");
}

// get the range parameters for the discontinuous distributions (it's all the same for these)
function getDiscontRangeParameters () {
	options['min'] = getValue ("min");
	options['max'] = getValue ("max");
	options['n'] = options['max'] - options['min'] + 1;
}

function doPrintout (full) {
	var plot_adds = "";
	var fun = getValue ("function");
	var _log = (getValue ("log") == 1);

	var is_density = "";
	var label = "";
	var tail_option = "";
	var tail_label = "";
	var log_option = "";
	if (fun == "d") {
		is_density = true;
		label = "density";
		tail_option = "";
		tail_label = "";
		if (_log) log_option = ", log=TRUE";
	} else {
		is_density = false;
		label = "distribution";
		if (getValue("lower") == "1") {
			tail_option = ", lower.tail = TRUE";
			tail_label = ", \"Tail\",\"Lower\"";
		} else {
			tail_option = ", lower.tail = FALSE";
			tail_label = ", \"Tail\",\"Upper\"";
		}
		if (_log) log_option = ", log.p=TRUE";
	}

	var log_label = ', "Scale", "normal"';
	if (_log) log_label = ', "Scale", "logarithmic"';

	options = new Array();
	options['is_density'] = is_density;
	options['label'] = label;
	options['tail_option'] = tail_option;
	options['tail_label'] = tail_label;
	options['log_option'] = log_option;
	options['log_label'] = log_label;

	getParameters ();

	if (full) {
		doHeader ();
		echo ('\n');
		echo ('rk.graph.on ()\n');
	}

	echo ('try ({\n');
	echo ('	curve (');
	doFunCall ();
	echo (', from=' + options['min'] + ', to=' + options['max'] + ', n=' + options['n'] + getValue ("plotoptions.code.printout") + ')\n');

	plot_adds = getValue ("plotoptions.code.calculate");
	if (plot_adds.length > 0) {
		echo ('\n');
		printIndented ("\t", plot_adds);
	}
	echo ('})\n');
	if (full) {
		echo ('rk.graph.off ()\n');
	}
}

