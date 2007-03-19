<?
function preprocess () { ?>
cor.graph <- function(x) {
	panel.cor <- function(x, y, digits=<? getRK ("digits"); ?>, cex.cor, use="<? getRK ("use"); ?>", method="<? getRK ("method"); ?>", scale=<? getRK ("scale"); ?>) {
		usr <- par("usr"); on.exit(par(usr))
		par(usr = c(0, 1, 0, 1))
		r <- abs(cor(x, y, use=use, method=method))
		txt <- format(c(r, 0.123456789), digits=digits)[1]
		if(missing(cex.cor)) cex <- 0.8/strwidth(txt)
	
		test <- cor.test(x,y, use=use, method=method)
		Signif <- symnum(test$p.value, corr = FALSE, na = FALSE,
				cutpoints = c(0, 0.001, 0.01, 0.05, 0.1, 1),
				symbols = c("***", "**", "*", ".", " "))

		if(scale) text(0.5, 0.5, txt, cex = cex * r)
		else text(0.5, 0.5, txt, cex = cex)
		text(.8, .8, Signif, cex=cex, col=2)
	}

	pairs(x, lower.panel=panel.smooth, upper.panel=panel.cor)
}
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
	$vars = str_replace ("\n", ",", trim (getRK_val ("x"))) ;
?>
data <- data.frame (<? echo ($vars); ?>)

<? 
	if ($final) { ?>
rk.header ("Correlation Matrix Plot", parameters=list ("Method", "<? getRK ("method"); ?>", "Exclusion", "<? getRK ("use"); ?>", "Precision", "<? getRK ("digits"); ?> digits", "Scale text", "<? getRK ("scale"); ?>"))

rk.graph.on ()
<?
} ?>
try ({
	cor.graph (data)
})
<? 
	if ($final) { ?>
rk.graph.off ()

print("Signif. codes:  0 '***', 0.001 '**', 0.01 '*', 0.05 '.'', 0.1 ' ', 1")
<?  }
}
?>
