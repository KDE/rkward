<?
function preprocess () {
}
	
function calculate () {
}

function printout () {
	doPrintout (true);
}
	
function cleanup () { ?>
rm (list=grep ("^rk.temp", ls (), value=TRUE))
<?
}
	
function preview () {
	preprocess ();
	calculate ();
	doPrintout (false);
	cleanup ();
}
	
function doPrintout ($final) {
	$vars = str_replace ("\n", ",", trim (getRK_val ("x"))) ;
	$labels = (getRK_val ("labels")=="TRUE");
?>
rk.temp.x <- (<? echo ($vars); ?>)
if(is.factor(rk.temp.x)) {
	rk.temp.x <- summary(rk.temp.x)
} else {
	# barplot is a bit picky about attributes, so we need to convert to vector explicitely
	if(!is.matrix(rk.temp.x)) rk.temp.x <- as.vector(rk.temp.x)
}

<? 
	if ($final) { ?>
rk.header ("Barplot", parameters=list ("Rainbow colors", "<? getRK ("rainbow"); ?>", "Beside", "<? getRK ("beside"); ?>", "Legend", "<? getRK ("legend"); ?>"))

rk.graph.on ()
<?	}
?>
try ({
<?	if (getRK_val ("beside") == "TRUE") { ?>
<?		if ($labels) { ?>
# adjust the range so that the labels will fit
rk.temp.range <- range (rk.temp.x, na.rm=TRUE) * 1.2
if (rk.temp.range[1] > 0) rk.temp.range[1] <- 0
if (rk.temp.range[2] < 0) rk.temp.range[2] <- 0
<?		} ?>
rk.temp.barplot <- barplot((rk.temp.x)<? if (getRK_val ("rainbow")=="TRUE") { ?>, col=rainbow( if(is.matrix(rk.temp.x)) dim(rk.temp.x) else length(rk.temp.x)) <? } ?>, beside=<? getRK ("beside"); ?>, legend.text=<? getRK ("legend"); ?><? if ($labels) echo (", ylim = rk.temp.range"); ?>)
<?		if ($labels) { ?>
text(rk.temp.barplot, rk.temp.x, labels=rk.temp.x, pos=<? getRK ("place"); ?>, offset=.5)
<?		}
	} else { ?>
barplot((rk.temp.x)<? if (getRK_val ("rainbow")=="TRUE") { ?>, col=rainbow( if(is.matrix(rk.temp.x)) dim(rk.temp.x) else length(rk.temp.x)) <? } ?>, legend.text=<? getRK ("legend"); ?>)
<?	}
	?>
})
<?	if ($final) { ?>
rk.graph.off ()
<? }
}
?>
