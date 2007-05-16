<?
function preprocess () {
?>
require(xtable)
<?
}
	
function calculate () {
	$x = getRK_val ("x") ;
	$y = "substitute (" . str_replace ("\n", "), substitute (", trim (getRK_val ("y"))) . ")";
?>
x <- <? echo ($x . "\n"); ?>
yvars <- list (<? echo ($y); ?>)
results <- list()
descriptions <- list ()

# calculate crosstabs
for (i in 1:length (yvars)) {
	yvar <- eval (yvars[[i]], envir=globalenv ())
	results[[i]] <- table(x, yvar)

	descriptions[[i]] <- list ('Dependent'=rk.get.description (<? echo ($x); ?>), 'Independent'=rk.get.description (yvars[[i]], is.substitute=TRUE))
}
<?
	if (getRK_val ("chisq") == "TRUE") { ?>

# calculate chisquares
chisquares <- list ()
for (i in 1:length (results)) {
	chisquares[[i]] <- chisq.test (results[[i]], simulate.p.value = <? getRK ("simpv");?><?if (getRK_val ("simpv") == "TRUE") { ?>,B=(<? getRK ("B"); ?>) <?}?>)
}
<?	}

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
	if ($final) { ?>
for (i in 1:length (results)) {
	rk.header ("Crosstabs", parameters=list ("Dependent", descriptions[[i]][['Dependent']], "Independent", descriptions[[i]][['Independent']]))
	rk.print (xtable (cbind (results[[i]]), digits=0))
<?		if (getRK_val ("chisq") == "TRUE") { ?>

	rk.header ("Pearson's Chi Square Test for Crosstabs", list ("Dependent", descriptions[[i]][['Dependent']], "Independent", descriptions[[i]][['Independent']], "Method", chisquares[[i]][["method"]]))
	rk.results (list ('Statistic'=chisquares[[i]][['statistic']], 'df'=chisquares[[i]][['parameter']], 'p'=chisquares[[i]][['p.value']]))
<?		}

		if (getRK_val ("barplot") == "TRUE") { ?>

	rk.header ("Barplot for Crosstabs", list ("Dependent", descriptions[[i]][['Dependent']], "Independent", descriptions[[i]][['Independent']]<? getRK ('barplot_embed.code.preprocess'); ?>))
	rk.graph.on ()
	try ({
<?			printIndented ("\t\t", getRK_val ('barplot_embed.code.printout')); ?>
	})
	rk.graph.off ()
<?		} ?>
}
<?	} else {
		// produce a single barplot of the first result ?>
i <- 1
<? getRK ('barplot_embed.code.printout'); ?>
<?	}

}
?>
