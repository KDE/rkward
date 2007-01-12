<?
	function preprocess () { ?>
		rk.temp.cor.graph <- function(x) {
			panel.cor <- function(x, y, digits=<? getRK ("digits"); ?>, cex.cor) {
    				usr <- par("usr"); on.exit(par(usr))
    				par(usr = c(0, 1, 0, 1))
    				r <- abs(cor(x, y, use="<? getRK ("use"); ?>", method="<? getRK ("method"); ?>"))
    				txt <- format(c(r, 0.123456789), digits=digits)[1]
    				txt <- paste(txt, sep="")
    				if(missing(cex.cor)) cex <- 0.8/strwidth(txt)
     		
     				test <- cor.test(x,y, use="<? getRK ("use"); ?>", method="<? getRK ("method"); ?>")
				Signif <- symnum(test$p.value, corr = FALSE, na = FALSE,
                  				cutpoints = c(0, 0.001, 0.01, 0.05, 0.1, 1),
                  				symbols = c("***", "**", "*", ".", " "))
    
    				if("<? getRK ("scale"); ?>"=="yes") text(0.5, 0.5, txt, cex = cex * r)
    				if("<? getRK ("scale"); ?>"=="no") text(0.5, 0.5, txt, cex = cex)
    				text(.8, .8, Signif, cex=cex, col=2)
			}
			pairs(x, 
  			lower.panel=panel.smooth, upper.panel=panel.cor)
		}
	<?		}
	
	function calculate () {
	}

	function printout () {
	$vars = str_replace ("\n", ",", trim (getRK_val ("x"))) ;
?>



	rk.temp.x<- data.frame (<? echo ($vars); ?>)

	rk.header ("Correlation Matrix Plot")

	rk.graph.on ()

	try ({
        	rk.temp.cor.graph (rk.temp.x)
	})

	rk.graph.off ()

	print("Signif. codes:  0 '***', 0.001 '**', 0.01 '*', 0.05 '.'', 0.1 ' ', 1")

<?
	}
	
	function cleanup () {
	?>
	rm(rk.temp.cor.graph,rk.temp.x)
	<?
	}
?>
