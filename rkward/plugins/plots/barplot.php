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
	$names_mode = getRK_val ("names_mode");
	$tabulate = getRK_val ("tabulate");

	if ($tabulate) {
		$tabulate_header = '"Tabulate", "Yes"';
	} else {
		$tabulate_header = '"Tabulate", "No"';
	}

	$barplot_header = getRK_val ("barplot_embed.code.preprocess");
	$barplot_main = getRK_val ("barplot_embed.code.printout");

?>
x <- <? echo ($var . "\n"); ?>
<?
	if ($tabulate) { ?>
x <- table(x, exclude=NULL)
<?      } else { ?>
# barplot is a bit picky about attributes, so we need to convert to vector explicitely
if(!is.matrix(x)) x <- as.vector(x)
<?	}

	if ($names_mode == "rexp") {
		echo ("names(x) <- " . getRK_val ("names_exp") . "\n");
	} else if ($names_mode == "custom") {
		echo ("names(x) <- c (\"" . str_replace (";", "\", \"", trim (getRK_val ("names_custom"))) . "\")\n");
	}

	if ($final) { ?>
rk.header ("Barplot", parameters=list ("Variable", rk.get.description (<? echo ($var); ?>), <? echo ($tabulate_header . $barplot_header); ?>))

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
