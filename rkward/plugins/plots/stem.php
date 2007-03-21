<?
function preprocess () {
}

function calculate () {
}

function printout () {
	$x = getRK_val ("x");
	$scale = getRK_val ("scale");
	$width = getRK_val ("width");
	$atom = getRK_val ("atom");
?>
rk.header ("Stem-and-Leaf Plot",
	parameters=list ("Variable", paste (rk.get.description (<? echo ($x); ?>)), "Plot Length", "<? echo ($scale); ?>","Plot Width", "<? echo ($width); ?>", "Tolerance", "<? echo ($atom); ?>"))

rk.print.literal(capture.output(stem(<? echo ($x); ?>, scale = <? echo ($scale); ?>, width = <? echo ($width); ?>, atom = <? echo ($atom); ?>)))
<?
}
?>
