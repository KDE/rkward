<?
function makeCodes () {
	global $histcalcoptions;
	global $histplotoptions;
	global $headeroptions;

	$histcalcoptions = ", breaks=";
	$histplotoptions = "";
	$headeroptions = "";
	$varname = getRK_val ("varname");

	$histbreaks = getRK_val ("histbreaksFunction");
	$headeroptions .= ', "Break points", "';
	if ($histbreaks == "cells") {
		$histcalcoptions .= getRK_val ("histbreaks_ncells");
		$headeroptions .= 'Approximately ' . getRK_val ("histbreaks_ncells") . ' cells"';
	} else if ($histbreaks == "int") {
		$histcalcoptions .= "seq (floor (min (" . $varname . ", na.rm=TRUE))-0.5, ceiling (max (" . $varname . ", na.rm=TRUE))+0.5)";
		$headeroptions .= 'Integers"';
	}
	else if ($histbreaks == "vec") {
		$histcalcoptions .= "(function(x) {y = extendrange(x,f=0.1); seq(from=y[1], to=y[2], length=" . getRK_val ("histbreaks_veclength") . ")})(" . $varname . ")";
		$headeroptions .= 'Equally spaced vector of length ' . getRK_val ("histbreaks_veclength") . '"';
	} else {
		$histcalcoptions .= "\"" . $histbreaks . "\"";
		$headeroptions .= $histbreaks . '"';
	}

	$right = getRK_val ("rightclosed");
	if (!$right) {
		$headeroptions .= ', "Right closed", "FALSE"';
		$histcalcoptions .= ", right=FALSE";
	} else {
		$headeroptions .= ', "Right closed", "TRUE"';
	}

	$inclowest = getRK_val ("include_lowest");
	if (!$inclowest) {
		$headeroptions .= ', "Include in lowest cell", "FALSE"';
		$histcalcoptions .= ", include.lowest=FALSE";
	} else {
		$headeroptions .= ', "Include in lowest cell", "TRUE"';
	}

	$freq = getRK_val ("freq");
	if (!$freq) {
		$histplotoptions .= ", freq=FALSE";
		$headeroptions .= ', "Scale", "Density"';
	} else {
		$headeroptions .= ', "Scale", "Frequency"';
	}

	$addbars = getRK_val ("addtoplot");
	if ($addbars) $histplotoptions .= ", add=TRUE";

	$labels = getRK_val ("barlabels");
	if ($labels) $histplotoptions .= ", labels=TRUE";

	$histlty = getRK_val ("histlinetype");
	$histplotoptions .= ", lty=" . "\"" . $histlty . "\"";

	if ($histlty != "blank") {
		$density = getRK_val ("density");
		$histplotoptions .= ", density=" . $density;
		if ($density > 0) $histplotoptions .= ", angle=" . getRK_val ("angle");
		if (getRK_val ("doborder")) $histbordercol = getRK_val ("histbordercol.code.printout");
		else $histbordercol = ", border=FALSE";
	}

	$histfillcol = "";
	if (getRK_val ("usefillcol")) $histfillcol = getRK_val ("histfillcol.code.printout");

	$histplotoptions .= $histbordercol . $histfillcol;
}

function preprocess () {
	global $headeroptions;

	makeCodes();

	echo ($headeroptions);
}

function calculate () {
	global $histcalcoptions;

	// makeCodes() has already run

	echo ($histcalcoptions);
}

function printout () {
	global $histplotoptions;

	// makeCodes() has already run

	echo ($histplotoptions);
}

function cleanup () {
}
?>
