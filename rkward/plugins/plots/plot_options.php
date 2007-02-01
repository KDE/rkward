<?
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

	$xlab = getRK_val ("xlab");
	if (($xlab != "") && (getRK_val ("xlabisexp") != "1")) {
		$xlab = "\"" . $xlab . "\"";
	}
	if ($xlab != "") $xlab = ", xlab=" . $xlab;

	$xminvalue = getRK_val ("xminvalue");
	$xmaxvalue = getRK_val ("xmaxvalue");
	if (($xminvalue != "") || ($xmaxvalue != "")) {
		$xlim = ", xlim=c (";
		if ($xminvalue == "") $xlim .= "min (" . $xvar . ")";
		else $xlim .= $xminvalue;
		$xlim .= ", ";
		if ($xmaxvalue == "") $xlim .= "max (" . $xvar . ")";
		else $xlim .= $xmaxvalue;
		$xlim .= ")";
	}


	// same for Y axis
	$yaxt = getRK_val ("yaxt");
	if ($yaxt != ""){
		$yaxt = ", yaxt=\"" . $yaxt . "\"";
	}
	$log .= getRK_val ("ylog");

	$ylab = getRK_val ("ylab");
	if (($ylab != "") && (getRK_val ("ylabisexp") != "1")) {
		$ylab = "\"" . $ylab . "\"";
	}
	if ($ylab != "") $ylab = ", ylab=" . $ylab;
	$yminvalue = getRK_val ("yminvalue");
	$ymaxvalue = getRK_val ("ymaxvalue");
	if (($yminvalue != "") || ($ymaxvalue != "")) {
		$ylim = ", ylim=c (";
		if ($yminvalue == "") $ylim .= "min (" . $yvar . ")";
		else $ylim .= $yminvalue;
		$ylim .= ", ";
		if ($ymaxvalue == "") $ylim .= "max (" . $yvar . ")";
		else $ylim .= $ymaxvalue;
		$ylim .= ")";
	}


	// final touches
	if ($log != "") $log = ", log=\"" . $log . "\"";

	$type = getRK_val ("pointtype");
	if (!empty ($type)) $type = ", type=\"" . $type . "\"";

	///Begin of Additions by Stefan Roediger
	//add a main (on top) to the plot
	$main = getRK_val ("main");
	if (($main != "") && (getRK_val ("mainisexp") != "1")) {
		$main = "\"" . $main . "\"";
	}
	if ($main != "") $main = ", main=" . $main;
	
	//add a subtitle (at bottom) to the plot
	$sub = getRK_val ("sub");
	if (($sub != "") && (getRK_val ("subisexp") != "1")) {
		$sub = "\"" . $sub . "\"";
	}
	if ($sub != "") $sub = ", sub=" . $sub;

	//define the aspect y/x of the plot
	$asp = getRK_val ("asp");
	if ($asp != 0) $asp = ", asp=" . $asp;
	else $asp = "";
	///End of Additions by Stefan Roediger

	// make option string
	$options = $type . $xaxt . $yaxt . $log . $xlim . $ylim . $xlab . $ylab . $main . $sub . $asp;

	echo ($options);
}

function cleanup () {
}
?>