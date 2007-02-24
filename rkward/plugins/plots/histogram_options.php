<?
function preprocess () {
}

function calculate () {
}

function printout () {
		$histbreaksoptions = ", breaks=";
		$histoptions = "";
		$varname = getRK_val ("varname");

		$histbreaks = getRK_val ("histbreaksFunction");
		if ($histbreaks == "cells") $histbreaksoptions .= getRK_val ("histbreaks_ncells");
		else if ($histbreaks == "int") $histbreaksoptions .= "seq (floor (min (" . $varname . ", na.rm=TRUE))-0.5, ceiling (max (" . $varname . ", na.rm=TRUE))+0.5)";
		else if ($histbreaks == "vec") {
			$histbreaksoptions .= "(function(x) {y = extendrange(x,f=0.1); seq(from=y[1], to=y[2], length=" . getRK_val ("histbreaks_veclength") . ")})(" . $varname . ")";
		}
		else $histbreaksoptions .= "\"" . $histbreaks . "\"";

		$addbars = getRK_val ("addtoplot");
		if ($addbars) $histoptions .= ", add=TRUE";

		$freq = getRK_val ("freq");
		if (!$freq) $histoptions .= ", freq=FALSE";

		$labels = getRK_val ("barlabels");
		if ($labels) $histoptions .= ", labels=TRUE";

		$right = getRK_val ("rightclosed");
		if (!$right) $histoptions .= ", right=FALSE";

		$inclowest = getRK_val ("include_lowest");
		if (!$inclowest) $histoptions .= ", include.lowest=FALSE";

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
		echo ($histbreaksoptions . "HIST_STRING_SPLITTER" . $histoptions);
}

function cleanup () {
}
?>
