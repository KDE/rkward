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

	echo ($options);
}
?>
