<?
function preprocess () {
?>
yrange <- range (<? getRK("x"); ?>, na.rm=TRUE)
<?
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
	$x = getRK_val ("x");
	
	if ($final) { ?>
rk.header ("Empirical Cumulative Distribution Function", list ("Variable", rk.get.description (<? echo ($x); ?>), "Minimum", yrange[1], "Maximum", yrange[2]))

rk.graph.on ()
<?	} ?>
try ({
	plot.ecdf (<? echo ($x); ?>, <? getRK ("stepfun_options.code.printout"); ?><? getRK ("plotoptions.code.printout"); ?>)
<?	if (getRK_val ("th_pnorm")) { ?>
	curve (pnorm, from=yrange[1], to=yrange[2], add=TRUE, <? getRK ("col_thnorm.code.printout"); ?>)
<?	}
	if (getRK_val ("rug")) { ?>
	rug (<? echo ($x); ?>, <? getRK ("ticksize"); ?>, <? getRK ("lwd"); ?>, <? getRK ("side"); ?><? getRK ("col_rug.code.printout"); ?>)
<?	} ?>
})
<?	if ($final) { ?>
rk.graph.off ()
<?	}
}

?>
