<?
function preprocess () {
?>
require(car)
<?
}

function calculate () {
}

function printout () {
	doPrintout (true);
}

function cleanup () {
?>
rm(rk.temp.x)
<?
}

function preview () {
	preprocess ();
	calculate ();
	doPrintout (false);
	cleanup ();
}

function doPrintout ($final) {
	$vars = str_replace ("\n", ",", trim (getRK_val ("x"))) ;
?>
rk.temp.x <- data.frame (<? echo ($vars); ?>)

<? 	if ($final) { ?>
rk.header ("Scatterplot Matrix", parameters=list ("Diagonal Panels", "<? getRK("diag") ?>", "Plot points", "<? getRK ("plot_points"); ?>", "Smooth", "<? getRK ("smooth"); ?>", "Ellipses", "<? getRK ("ellipse"); ?> at 0.5 and 0.9 levels."))

rk.graph.on ()
<?	} ?>
try (scatterplot.matrix(rk.temp.x, diagonal="<? getRK("diag") ?>", plot.points=<? getRK ("plot_points"); ?>, smooth=<? getRK ("smooth"); ?>, ellipse=<? getRK ("ellipse"); ?>))
<?	if ($final) { ?>
rk.graph.off ()
<?	}
}
?>
