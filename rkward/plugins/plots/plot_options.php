<?
function prepareLabel ($labelname) {
	$quoted = (getRK_val ($labelname . "isquote") == "1");
	$label = getRK_val ($labelname);
	if ($label == "") {
		$label = getRK_val ("default_" . $labelname);
		$quoted = true;
	}
	if (($label != "") && ($quoted)) $label = "\"" . strtr ($label, array ('"' => '\\"')) . "\"";
	if ($label != "") $label = ", " . $labelname . "=" . $label;

	return $label;
}

function preprocess () {
}

function calculate () {
}

function printout () {
	$log; $xaxt; $yaxt; $xlim; $ylim; $xlab; $ylab; $main; $sub;

	$xvars = split ("\n", getRK_val ("xvar"));
	if (count ($xvars) > 1) {
		$xvar = "c (" . join (", ", $xvars) . ")";
	} else {
		$xvar = $xvars[0];
	}
	$yvars = getRK_val ("yvar");
	if (count ($yvars) > 1) {
		$yvar = "c (" . join (", ", $yvars) . ")";
	} else {
		$yvar = $yvars[0];
	}

	if ($yvar == "") {
		$yvar = "1:length (" . $xvar .")";
	} else if ($xvar == "") {	// don't replace both at the same time, even if both are empty
		$xvar = "1:length (" . $yvar .")";
	}

	// X axis
	$xaxt = getRK_val ("xaxt");
	if ($xaxt != "") {
		$xaxt = ", xaxt=\"" . $xaxt . "\"";
	}
	$log .= getRK_val ("xlog");

	$xlab = prepareLabel ("xlab");
	$xminvalue = getRK_val ("xminvalue");
	$xmaxvalue = getRK_val ("xmaxvalue");
	if (($xminvalue != "") || ($xmaxvalue != "")) {
		$xlim = ", xlim=c (";
		if (($xminvalue == "") && ($xvar != "")) $xlim .= "min (" . $xvar . ")";
		else $xlim .= $xminvalue;
		$xlim .= ", ";
		if (($xmaxvalue == "") && ($xvar != "")) $xlim .= "max (" . $xvar . ")";
		else $xlim .= $xmaxvalue;
		$xlim .= ")";
	}


	// same for Y axis
	$yaxt = getRK_val ("yaxt");
	if ($yaxt != ""){
		$yaxt = ", yaxt=\"" . $yaxt . "\"";
	}
	$log .= getRK_val ("ylog");

	$ylab = prepareLabel ("ylab");
	$yminvalue = getRK_val ("yminvalue");
	$ymaxvalue = getRK_val ("ymaxvalue");
	if (($yminvalue != "") || ($ymaxvalue != "")) {
		$ylim = ", ylim=c (";
		if (($yminvalue == "") && ($yvar != "")) $ylim .= "min (" . $yvar . ")";
		else $ylim .= $yminvalue;
		$ylim .= ", ";
		if (($ymaxvalue == "") && ($yvar != "")) $ylim .= "max (" . $yvar . ")";
		else $ylim .= $ymaxvalue;
		$ylim .= ")";
	}


	// final touches
	if ($log != "") $log = ", log=\"" . $log . "\"";

	$type = getRK_val ("pointtype");
	if (empty ($type)) $type = getRK_val ("default_pointtype");
	if (!empty ($type)) $type = ", type=\"" . $type . "\"";

	//color of points / lines
	$col = getRK_val ("pointcolor.code.printout");

	// main and subtitle to the plot
	$main = prepareLabel ("main");
	$sub = prepareLabel ("sub");

	//define the aspect y/x of the plot
	$asp = getRK_val ("asp");
	if ($asp != 0) $asp = ", asp=" . $asp;
	else $asp = "";

	// make option string
	$options = $type . $col . $xaxt . $yaxt . $log . $xlim . $ylim . $xlab . $ylab . $main . $sub . $asp;

	echo ($options);
}

function cleanup () {
}
?>