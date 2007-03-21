<?
function preprocess () {
}

function calculate () {
	$x = str_replace ("\n", ",", trim (getRK_val ("x"))) ;
	$y = str_replace ("\n", ",", trim (getRK_val ("y"))) ;
	
	if (getRK_val ("manual_type") == "true") {
		$type = getRK_val ("custom_type");
	} else {
		$type = "c ('" . getRK_val ("pointtype") . "')";
	}
	$col = getRK_val ("col");
	$pch = getRK_val ("pch");
	$cex = getRK_val ("cex");
?>

<? #input ?>
Xvars <- list(<? echo ($x) ;?>)
Yvars <- list(<? echo ($y) ;?>)

<? # verification (is this needed?) ?>
if (length(Xvars) != length(Yvars)) {
	stop("Unequal number of X and Y variables given")
}

# find range of X/Y values needed
Xrange <- range (c (Xvars), na.rm=TRUE)
Yrange <- range (c (Yvars), na.rm=TRUE)

type <- rep (<? echo ($type); ?>, length.out=length (Xvars));
col <- rep (<? echo ($col); ?>, length.out=length (Xvars));
cex <- rep (<? echo ($cex); ?>, length.out=length (Xvars));
pch <- rep (<? echo ($pch); ?>, length.out=length (Xvars));
<?
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
rk.graph.on()

<?	} ?>
try ({
	# make frame and axes
	plot(Xrange, Yrange, type="n"<? getRK ("plotoptions.code.printout"); ?>)
	
	# plot variables one X/Y pair at a time
	for (i in 1:length(Xvars)) {
		points (
			Xvars[[i]],
			Yvars[[i]],
			type = type[[i]],
			col = col[[i]],
			cex = cex[[i]],
			pch = pch[[i]]
		)
	}
})
<?	if ($final) { ?>

rk.graph.off()
<?	}
}
?>
