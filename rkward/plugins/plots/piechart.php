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
	$vars = getRK_val ("x");
	$names = getRK_val ("names");
	$tabulate= getRK_val ("tabulate")=="TRUE";
	$radius = getRK_val ("radius");
	$col = getRK_val ("colors");
	$clockwise = getRK_val ("clockwise");

	$options = "";
	$options .= ", clockwise =" . $clockwise;
	if ($radius != 0.8) $options .= ", radius=" . $radius;
	if ($col == "rainbow") $options .= ", col=rainbow (if(is.matrix(x)) dim(x) else length(x))";
	else if ($col == "grayscale") $options .= ", col=gray.colors (if(is.matrix(x)) dim(x) else length(x))";
	$options .= getRK_val ("plotoptions.code.printout");

	$plotpre = getRK_val ("plotoptions.code.preprocess");
	$plotpost = getRK_val ("plotoptions.code.calculate");

if($tabulate) {?>
x <- table (<? echo ($vars); ?>, exclude=NULL)
<?      } else { ?>
x <- <? echo ($vars); ?>

if (!is.numeric (x)) {
	warning ("Data may not be numeric, but proceeding as requested.\nDid you forget to check the tabulate option?")
}
<?      } ?>

<?	if ($final) { ?>
rk.header ("Pie chart", list ("Variable", rk.get.description (<? echo ($vars); ?>)))

rk.graph.on ()
<?	}
?>
try ({
<?	if (!empty ($plotpre)) printIndented ("\t", $plotpre); ?>
	names (x) <- <? echo ($names); ?>;
	pie(x<? echo ($options); ?>)
<?	if (!empty ($plotpost)) printIndented ("\t", $plotpost); ?>
})
<?	if ($final) { ?>
rk.graph.off ()
<? }
}
?>
