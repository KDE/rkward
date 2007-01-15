<?
	function preprocess () {
?>
require(car)
<?
	}
	
	function calculate () {
	}
	
	function printout () {
	$vars = str_replace ("\n", ",", trim (getRK_val ("x"))) ;
?>

rk.temp.x<- data.frame (<? echo ($vars); ?>)

rk.header ("Correlation Matrix Plot", parameters=list ("Diagonal Panels", "<? getRK("diag") ?>", "Plot points", "<? getRK ("plot_points"); ?>", "Smooth", "<? getRK ("smooth"); ?>", "Ellipses", "<? getRK ("ellipse"); ?> at 0.5 and 0.9 (normal) probability or confidence levels."))

rk.graph.on ()

scatterplot.matrix(rk.temp.x, diagonal= "<? getRK("diag") ?>", plot.points=<? getRK ("plot_points"); ?>, smooth=<? getRK ("smooth"); ?>, ellipse=<? getRK ("ellipse"); ?>)

rk.graph.off ()

<?
	}
	
	function cleanup () {
?>
rm(rk.temp.x)
<?
	}
?>
