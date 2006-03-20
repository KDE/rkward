<?
function preprocess () {
}

function calculate () {
}

function printout () {
	$log; $xaxt; $yaxt; $xlim; $ylim; $xlab; $ylab;

	$xvar = getRK_val ("xvar");
	$yvar = getRK_val ("yvar");
	if ($yvar == "") {
		$yvar = "1:length (" . $xvar .")";
	}

	// X axis
	$xaxt = getRK_val ("xaxt");
	$have_x_axis = ($xaxt == "");
	if ($have_x_axis) {
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
	} else {
		$xaxt = ", xaxt=\"" . $xaxt . "\"";
	}

	// same for Y axis
	$yaxt = getRK_val ("yaxt");
	$have_y_axis = ($yaxt == "");
	if ($have_y_axis) {
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
			else $ylim .= $yminvalue . ", ";
			$ylim .= ", ";
			if ($ymaxvalue == "") $ylim .= "max (" . $yvar . ")";
			else $ylim .= $ymaxvalue;
			$ylim .= ")";
		}
	} else {
		$yaxt = ", yaxt=\"" . $yaxt . "\"";
	}

	// final touches
	if ($log != "") $log = ", log=\"" . $log . "\"";

	// make option string
	$options = $xaxt . $yaxt . $log . $xlim . $ylim . $xlab . $ylab;

	echo ($options);
}

function cleanup () {
}
?>