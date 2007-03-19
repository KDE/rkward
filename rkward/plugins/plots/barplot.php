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
	$var = getRK_val ("x") ;
	$labels = getRK_val ("labels")=="TRUE";
	$tabulate= getRK_val ("tabulate")=="TRUE";
	
?>
x <- <? echo ($var); ?>
<?
	if($tabulate) { ?>
x <- table(x, exclude=NULL)
<?      } else { ?>
# barplot is a bit picky about attributes, so we need to convert to vector explicitely
if(!is.matrix(x)) x <- as.vector(x)
<? }

	if ($final) { ?>
rk.header ("Barplot", parameters=list ("Rainbow colors", "<? getRK ("rainbow"); ?>", "Beside", "<? getRK ("beside"); ?>", "Legend", "<? getRK ("legend"); ?>"))

rk.graph.on ()
<?	}
?>
try ({
<?	if (getRK_val ("beside") == "TRUE") { ?>
<?		if ($labels) { ?>
	# adjust the range so that the labels will fit
	yrange <- range (x, na.rm=TRUE) * 1.2
	if (yrange[1] > 0) yrange[1] <- 0
	if (yrange[2] < 0) yrange[2] <- 0
<?		} ?>
	bplot <- barplot(x<? if (getRK_val ("rainbow")=="TRUE") { ?>, col=rainbow( if(is.matrix(x)) dim(x) else length(x))<? } ?>, beside=<? getRK ("beside"); ?>, legend.text=<? getRK ("legend"); ?><? if ($labels) echo (", ylim = yrange"); ?>)
<?		if ($labels) { ?>
	text(bplot, x, labels=x, pos=<? getRK ("place"); ?>, offset=.5)
<?		}
	} else { ?>
	barplot(x<? if (getRK_val ("rainbow")=="TRUE") { ?>, col=rainbow( if(is.matrix(x)) dim(x) else length(x))<? } ?>, legend.text=<? getRK ("legend"); ?>)
<?	}
	?>
})
<?	if ($final) { ?>
rk.graph.off ()
<? }
}
?>
