<?
function makeCodes () {
	global $histbreaksoptions;
	global $histoptions;
	global $headeroptions;

	$histbreaksoptions = ", breaks=";
	$histoptions = "";
	$headeroptions = "";
	$varname = getRK_val ("varname");

	$histbreaks = getRK_val ("histbreaksFunction");
	$headeroptions .= ', Breaks, "';
	if ($histbreaks == "cells") {
		$histbreaksoptions .= getRK_val ("histbreaks_ncells");
		$header_options .= getRK_val ("histbreaks_ncells") . 'cells"';
	} else if ($histbreaks == "int") {
		$histbreaksoptions .= "seq (floor (min (" . $varname . ", na.rm=TRUE))-0.5, ceiling (max (" . $varname . ", na.rm=TRUE))+0.5)";
		$header_options .= 'integers"';
	}
	else if ($histbreaks == "vec") {
		$histbreaksoptions .= "(function(x) {y = extendrange(x,f=0.1); seq(from=y[1], to=y[2], length=" . getRK_val ("histbreaks_veclength") . ")})(" . $varname . ")";
		$header_options .= 'vector length ' . getRK_val ("histbreaks_veclength") . '"';
	} else {
		$histbreaksoptions .= "\"" . $histbreaks . "\"";
		$header_options .= $histbreaks . '"';
	}

	$addbars = getRK_val ("addtoplot");
	if ($addbars) $histoptions .= ", add=TRUE";

	$freq = getRK_val ("freq");
	if (!$freq) {
		$histoptions .= ", freq=FALSE";
		$headeroptions .= ', Scale, "Density"';
	} else {
		$headeroptions .= ', Scale, "Frequency"';
	}

	$labels = getRK_val ("barlabels");
	if ($labels) $histoptions .= ", labels=TRUE";

	$right = getRK_val ("rightclosed");
	if (!$right) {
		$headeroptions .= ', "Right closed", "FALSE"';
		$histoptions .= ", right=FALSE";
	} else {
		$headeroptions .= ', "Right closed", "TRUE"';
	}

	$inclowest = getRK_val ("include_lowest");
	if (!$inclowest) {
		$headeroptions .= ', "Include in lowest cell", "FALSE"';
		$histoptions .= ", include.lowest=FALSE";
	} else {
		$headeroptions .= ', "Include in lowest cell", "TRUE"';
	}

	$histlty = getRK_val ("histlinetype");
	$histoptions .= ", lty=" . "\"" . $histlty . "\"";

	if ($histlty != "blank") {
		$density = getRK_val ("density");
		$histoptions .= ", density=" . $density;
		if ($density > 0) $histoptions .= ", angle=" . getRK_val ("angle");
		if (getRK_val ("doborder")) $histbordercol = getRK_val ("histbordercol.code.printout");
		else $histbordercol = ", border=FALSE";
	}

	$histfillcol = "";
	if (getRK_val ("usefillcol")) $histfillcol = getRK_val ("histfillcol.code.printout");

	$histoptions .= $histbordercol . $histfillcol;
}

function preprocess () {
	global $headeroptions;

	makeCodes();

	echo ($headeroptions);
}

function calculate () {
	global $histbreaksoptions;

	// makeCodes() has already run

	echo ($histbreaksoptions);
}

function printout () {
	global $histoptions;

	// makeCodes() has already run

	echo ($histoptions);
}

function cleanup () {
}
?>
