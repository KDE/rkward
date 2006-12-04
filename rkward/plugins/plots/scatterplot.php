<?
function preprocess () {
?>
	rk.temp <- list ()
<?
}

function calculate () {
	$x = str_replace ("\n", ",", trim (getRK_val ("x"))) ;
	$y = str_replace ("\n", ",", trim (getRK_val ("y"))) ;
	
	/** fetch some values which are needed in more than one place, to avoid mulitple transfer */
	$type = getRK_val ("type");
	$typeCusto = getRK_val ("typeCusto");
	if ($type == "custoType") $type_string = $typeCusto;
	else $type_string = $type;
	$col = getRK_val ("col");
	$pch = getRK_val ("pch");
	$cex = getRK_val ("cex");
	if (getRK_val("isXaxis") == "1") $Xname = getRK_val ("Xname"); else $Xname = "";
	if (getRK_val("isYaxis") == "1") $Yname = getRK_val ("Yname"); else $Yname = "";
	if (getRK_val("isTitle") == "1") $main = getRK_val ("main"); else $main = "";
	if (getRK_val("isSub") == "1") $sub = getRK_val ("sub"); else $sub = "";
?>

<? #input ?>
rk.temp$Xvar <- list(<? echo ($x) ;?>)
rk.temp$Yvar <- list(<? echo ($y) ;?>)
rk.temp$Xval <- <? if (getRK_val("columns") == "custoCol" ) echo (getRK_val("Xscale") . "\n"); else echo ("c(" . $x . ")\n"); ?>
rk.temp$Yval <- <? if (getRK_val("rows") == "custoRow" ) echo (getRK_val("Yscale") . "\n"); else echo ("c(" . $y . ")\n"); ?>

<? # verification (chiant mais doit être fait)?>
rk.temp$ok <- TRUE
if (length(rk.temp$Xvar) != length(rk.temp$Yvar)) {
	rk.temp$ok <- FALSE ;
	stop("Unequal number of X and Y variables given")
}

# find range of X/Y values needed
rk.temp$Xdef <- c(min(rk.temp$Xval,na.rm=TRUE), max(rk.temp$Xval,na.rm=TRUE))
rk.temp$Ydef <- c(min(rk.temp$Yval,na.rm=TRUE), max(rk.temp$Yval,na.rm=TRUE))

rk.temp$type <- rep (<? echo ($type_string); ?>, length.out=length (rk.temp$Xvar));
rk.temp$col <- rep (<? echo ($col); ?>, length.out=length (rk.temp$Xvar));
rk.temp$cex <- rep (<? echo ($cex); ?>, length.out=length (rk.temp$Xvar));
rk.temp$pch <- rep (<? echo ($pch); ?>, length.out=length (rk.temp$Xvar));
<?
}

function printout () {
	
?>
if (!rk.temp$ok) stop ()

rk.graph.on()

try ({
	# make frame and axes
	plot(rk.temp$Xdef, rk.temp$Ydef, type="n", xlab = "<? echo ($Xname); ?>", ylab = "<? echo ($Yname); ?>", main = "<? echo ($main); ?>", sub = "<? echo ($sub); ?>", axes = <? getRK("axes") ;?>, log = "<? getRK("logX") ; getRK("logY") ; ?>")
	
	# plot variables one X/Y pair at a time
	for (rk.temp.iterator in 1:length(rk.temp$Xvar)) {
		points (
			rk.temp$Xvar[[rk.temp.iterator]],
			rk.temp$Yvar[[rk.temp.iterator]],
			type = rk.temp$type[[rk.temp.iterator]],
			col = rk.temp$col[[rk.temp.iterator]],
			cex = rk.temp$cex[[rk.temp.iterator]],
			pch = rk.temp$pch[[rk.temp.iterator]]
		)
	}
})

rk.graph.off()

<?
}

function cleanup () {
?>
rm(rk.temp)
rm(rk.temp.iterator)
<?
}
?>
