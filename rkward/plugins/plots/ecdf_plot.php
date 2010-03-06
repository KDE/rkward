<?
function preprocess () {
	$x = getRK_val ("x");
?>
yrange <- range (<? echo ($x); ?>, na.rm=TRUE)
<?
	if (getRK_val ("th_pnorm") && getRK_val ("adjust_th_pnorm")) { ?>
data.mean <- mean (<? echo ($x); ?>, na.rm=TRUE)
data.sd <- sd (<? echo ($x); ?>, na.rm=TRUE)
<?
	}
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
	curve (pnorm<? if (getRK_val ("adjust_th_pnorm")) echo " (x, mean=data.mean, sd=data.sd)"; ?>, from=yrange[1], to=yrange[2], add=TRUE, <? getRK ("col_thnorm.code.printout"); ?>)
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
