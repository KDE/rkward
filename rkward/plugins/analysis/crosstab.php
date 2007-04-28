<?
function preprocess () {
?>
require(xtable)
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
	$x = getRK_val ("x") ;
	$yvarsstring = join (", ", split ("\n", getRK_val ("y")));
	$labels = getRK_val ("labels")=="TRUE";
	
?>
x <- (<? echo ($x); ?>)
y <- cbind (<? echo ($yvarsstring); ?>)

for (i in 1:dim(y)[2]){
xy<-table(x,y[,i])
rk.header ("Crosstabs", list ("Dependent", rk.get.description (<? echo ($x); ?>), "Independent", rk.get.description (<? echo ($yvarsstring); ?>)[i]))

<?	if ($final) { ?>
rk.print(xtable(cbind(xy)))
<?	if (getRK_val ("chisq") == "TRUE") { ?>
rk.header ("Pearson's Chi Square Test for Crosstabs", list ("Dependent", rk.get.description (<? echo ($x); ?>), "Independent", rk.get.description (<? echo ($yvarsstring); ?>)[i], "Simulate p vlaue", "<? getRK ("simpv");?>", "Monte Carlo", "<? getRK ("monte"); ?>" <? if (getRK_val ("monte") == "TRUE") { ?>,  "Number of replicates", <? getRK ("B"); }?> ))
xsquared<-cbind(
chisq.test(xy, simulate.p.value = <? getRK ("simpv");?> <?if (getRK_val ("monte") == "TRUE") { ?>,B=(<? getRK ("B"); ?>) <?}?> )$statistic,
<?if (getRK_val ("simpv") == "FALSE") {?> chisq.test(xy, simulate.p.value = <? getRK ("simpv");?> <?if (getRK_val ("monte") == "TRUE") { ?>,B=(<? getRK ("B"); ?>) <?}?> )$parameter, <?}?>
chisq.test(xy, simulate.p.value = <? getRK ("simpv");?> <?if (getRK_val ("monte") == "TRUE") { ?>,B=(<? getRK ("B"); ?>) <?}?> )$p.value)
 colnames(xsquared)<-c("Statistic", <?if (getRK_val ("simpv") == "FALSE") {?> "df", <?}?> "p-value")
 rk.print(xtable(xsquared))
<? } ?>
<?	if (getRK_val ("barplot") == "TRUE") { ?>
rk.header ("Barplot for Crosstabs", list ("Dependent", rk.get.description (<? echo ($x); ?>), "Independent", rk.get.description (<? echo ($yvarsstring); ?>)[i] <? if (getRK_val ("barplot") == "TRUE") { ?> , "Rainbow colors", "<? getRK ("rainbow"); ?>", "Beside", "<? getRK ("beside"); ?>", "Legend", "<? getRK ("legend"); ?>"<? } ?>)) 
rk.graph.on ()
<?	} 
}
if (getRK_val ("barplot") == "TRUE") {
?>
try ({
par(bg="gray95") 
<?	if (getRK_val ("beside") == "TRUE") { ?>
<?		if ($labels) { ?>
# adjust the range so that the labels will fit
yrange <- range (xy, na.rm=TRUE) * 1.2
if (yrange[1] > 0) yrange[1] <- 0
if (yrange[2] < 0) yrange[2] <- 0
<?		} ?>
bplot <- barplot((xy)<? if (getRK_val ("rainbow")=="TRUE") { ?>, col=rainbow( if(is.matrix(xy)) dim(xy) else length(xy)) <? } ?>, beside=<? getRK ("beside"); ?>, legend.text=<? getRK ("legend"); ?><? if ($labels) echo (", ylim = yrange"); ?>)
<?		if ($labels) { ?>
text(bplot, xy, labels=xy, pos=<? getRK ("place"); ?>, offset=.5)
<?		}
	} else { ?>
barplot((xy)<? if (getRK_val ("rainbow")=="TRUE") { ?>, col=rainbow( if(is.matrix(xy)) dim(xy) else length(xy)) <? } ?>, legend.text=<? getRK ("legend"); ?>)
<?	}
	?>
})

<?	}
if ($final) { ?>
<?	if (getRK_val ("barplot") == "TRUE") { ?>
rk.graph.off ()
<? } 
} ?>
}
<? }
?>
