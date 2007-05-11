<?
function preprocess () {
}

function calculate () {
}

function printout () {
	doPrintout (true);
}
	
function preview () {
	preprocess ();
	calculate ();
	doPrintout (false);
}
	
function doPrintout ($final) {
	$var = getRK_val ("x");

	$barplot_header = getRK_val ("barplot_options.code.preprocess");
	$barplot_main = getRK_val ("barplot_options.code.printout");

?>
x <- <? echo ($var . "\n"); ?>
<?
	if ($tabulate) { ?>
x <- table(x, exclude=NULL)
<?      } else { ?>
# barplot is a bit picky about attributes, so we need to convert to vector explicitely
if(!is.matrix(x)) x <- as.vector(x)
<? }

	if ($final) { ?>
rk.header ("Barplot", parameters=list ("dummy", "dummy"<? echo ($barplot_header); ?>))

rk.graph.on ()
<?	}
?>
try ({
<?
	printIndented ("\t", $barplot_main);
?>
})
<?	if ($final) { ?>
rk.graph.off ()
<? }
}
?>
