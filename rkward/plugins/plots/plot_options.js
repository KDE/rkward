function prepareLabel (labelname) {
	var quoted = (getValue (labelname + "isquote") == "1");
	var label = getValue (labelname);
	if (label == "") {
		label = getValue ("default_" + labelname);
		quoted = false;
	}
	if ((label != "") && (quoted)) label = quote (label);
	if (label != "") label = ", " + labelname + "=" + label;

	return label;
}


function calculate () {
	if (getValue ("grid_enable") == "true") {
		echo (getValue ("grid_options.code.printout"));
	}
}

function printout () {
	var log = "";
	var xaxt = "";
	var yaxt = "";
	var xlim = "";
	var ylim = "";
	var xlab = "";
	var ylab = "";
	var main = "";
	var sub = "";
	var xvars = "";
	var xvar = "";
	var yvars = "";
	var yvar = "";
	var xminvalue = "";
	var xmaxvalue = "";
	var yminvalue = "";
	var ymaxvalue = "";
	var type = "";
	var col = "";
	var asp = "";
	var options = "";

	xvars = getValue ("xvar").split ("\n");
	if (xvars.length > 1) {
		xvar = "c (" + xvars.join (", ") + ")";
	} else {
		xvar = xvars[0];
	}
	yvars = getValue ("yvar").split ("\n");
	if (yvars.length > 1) {
		yvar = "c (" + xvars.join (", ") + ")";
	} else {
		yvar = yvars[0];
	}

	if (yvar == "") {
		yvar = "1:length (" + xvar +")";
	} else if (xvar == "") {	// don't replace both at the same time, even if both are empty
		xvar = "1:length (" + yvar +")";
	}

	// X axis
	xaxt = getValue ("xaxt");
	if (xaxt != "") {
		xaxt = ", xaxt=\"" + xaxt + "\"";
	}
	log += getValue ("xlog");

	xlab = prepareLabel ("xlab");
	xminvalue = getValue ("xminvalue");
	xmaxvalue = getValue ("xmaxvalue");
	if ((xminvalue != "") || (xmaxvalue != "")) {
		xlim = ", xlim=c (";
		if ((xminvalue == "") && (xvar != "")) xlim += "min (" + xvar + ")";
		else xlim += xminvalue;
		xlim += ", ";
		if ((xmaxvalue == "") && (xvar != "")) xlim += "max (" + xvar + ")";
		else xlim += xmaxvalue;
		xlim += ")";
	}


	// same for Y axis
	yaxt = getValue ("yaxt");
	if (yaxt != "") {
		yaxt = ", yaxt=\"" + yaxt + "\"";
	}
	log += getValue ("ylog");

	ylab = prepareLabel ("ylab");
	yminvalue = getValue ("yminvalue");
	ymaxvalue = getValue ("ymaxvalue");
	if ((yminvalue != "") || (ymaxvalue != "")) {
		ylim = ", ylim=c (";
		if ((yminvalue == "") && (yvar != "")) ylim += "min (" + yvar + ")";
		else ylim += yminvalue;
		ylim += ", ";
		if ((ymaxvalue == "") && (yvar != "")) ylim += "max (" + yvar + ")";
		else ylim += ymaxvalue;
		ylim += ")";
	}


	// final touches
	if (log != "") log = ", log=\"" + log + "\"";

	type = getValue ("pointtype");
	if (type == "") type = getValue ("default_pointtype");
	if (type != "") type = ", type=\"" + type + "\"";

	//color of points / lines
	col = getValue ("pointcolor.code.printout");

	// main and subtitle to the plot
	main = prepareLabel ("main");
	sub = prepareLabel ("sub");

	//define the aspect y/x of the plot
	asp = getValue ("asp");
	if (asp != 0) asp = ", asp=" + asp;
	else asp = "";

	// make option string
	options = type + col + xaxt + yaxt + log + xlim + ylim + xlab + ylab + main + sub + asp;

	echo (options);
}

