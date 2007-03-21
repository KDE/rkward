<?
function preprocess () {
}

function calculate () {
}

function printout () {
	$options = "";

	$addtoplot = getRK_val ("addtoplot");
	if ($addtoplot) $options .= ', add=TRUE';

	$lty = getRK_val ("linetype");
	if (!($lty == "")) $options .= ", lty=\"{$lty}\"";

	$verticals = getRK_val ("verticals");
	if ($verticals) {
		$options .= ', verticals=TRUE' . getRK_val ("col_vert.code.printout");
	} else $options .= ', verticals=FALSE';

	$do_points = getRK_val ("do_points");
	if ($do_points) {
		$options .= getRK_val ("col_points.code.printout");
	} else $options .= ', do.points=FALSE';

	$col_hor = getRK_val ("col_hor.code.printout");
	if (!($col_hor == "")) $options .= getRK_val ("col_hor.code.printout");

	if (getRK_val ("visible_col01line")) {
		$col_y0 = getRK_val ("col_y0.code.printout");
		$col_y1 = getRK_val ("col_y1.code.printout");
		if (($col_y0 != "") && ($col_y1 != "")) {
			$options .= ", col.01line=c({$col_y0},{$col_y1})";
		} elseif (($col_y0 != "") || ($col_y1 != "")) {
			$options .= ", col.01line={$col_y0}{$col_y1}";
		} // col.01line option to plot.ecdf()
	}

	echo ($options);
}
?>
