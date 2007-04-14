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
	$y = getRK_val ("y") ;
	$labels = getRK_val ("labels")=="TRUE";
	
?>
x <- (<? echo ($x); ?>)
y <- (<? echo ($y); ?>)

rk.header ("Crosstabs", list ("Variable", rk.get.description (<? echo ($x); ?>), "Group", rk.get.description (<? echo ($y); ?>)))
xy<-table(x,y)

<?	if ($final) { ?>
rk.print(xtable(cbind(xy)))
<?	if (getRK_val ("chisq") == "TRUE") { ?>
rk.header ("Pearson's Chi Square Test for Crosstabs", list ("Variable", rk.get.description (<? echo ($x); ?>), "Group", rk.get.description (<? echo ($y); ?>), "Monte Carlo", "<? getRK ("monte"); ?>" <? if (getRK_val ("monte") == "TRUE") { ?>,  "Number of replicates", <? getRK ("B"); }?> ))
xsquared<-cbind(
chisq.test(x,y <?if (getRK_val ("monte") == "TRUE") { ?>,B=(<? getRK ("B"); ?>) <?}?> )$statistic,
 chisq.test(x,y <?if (getRK_val ("monte") == "TRUE") { ?>,B=(<? getRK ("B"); ?>) <?}?> )$parameter,
 chisq.test(x,y <?if (getRK_val ("monte") == "TRUE") { ?>,B=(<? getRK ("B"); ?>) <?}?> )$p.value)
 colnames(xsquared)<-c("Statistic", "df", "p-value")
 rk.print(xtable(xsquared))
<? } ?>
<?	if (getRK_val ("barplot") == "TRUE") { ?>
rk.header ("Barplot for Crosstabs", list ("Variable", rk.get.description (<? echo ($x); ?>), "Group", rk.get.description (<? echo ($y); ?>) <?	if (getRK_val ("barplot") == "TRUE") { ?> , "Rainbow colors", "<? getRK ("rainbow"); ?>", "Beside", "<? getRK ("beside"); ?>", "Legend", "<? getRK ("legend"); ?>"<? } ?>)) 
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
}
}
?>
