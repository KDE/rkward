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
	$tabulate= getRK_val ("tabulate")=="TRUE";
	$radius = getRK_val ("radius");
	$angle = getRK_val ("angle");
	$angle_inc = getRK_val ("angle_inc");
	$density = getRK_val ("density");
	$density_inc = getRK_val ("density_inc");
	$col = getRK_val ("colors");
	$clockwise = getRK_val ("clockwise");
	$names_mode = getRK_val ("names_mode");

	$options = "";
	$options .= ", clockwise =" . $clockwise;
	if (($density >= 0) || ($density_inc != 0)) $options .= ", density =" . $density;
	if ($density_inc != 0) $options .= "+ $density_inc * 0:length (x)";
	if (($density > 0) || $density_inc != 0) {
		$options .= ", angle =" . $angle;
		if ($angle_inc != 0) $options .= "+ $angle_inc * 0:length (x)";
	}
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
<?	if (!empty ($plotpre)) printIndented ("\t", $plotpre);
	if ($names_mode == "rexp") {
		echo ("\tnames(x) <- " . getRK_val ("names_exp") . "\n");
	} else if ($names_mode == "custom") {
		echo ("\tnames(x) <- c (\"" . str_replace (";", "\", \"", trim (getRK_val ("names_custom"))) . "\")\n");
	}
?>
	pie(x<? echo ($options); ?>)
<?	if (!empty ($plotpost)) printIndented ("\t", $plotpost); ?>
})
<?	if ($final) { ?>
rk.graph.off ()
<? }
}
?>
