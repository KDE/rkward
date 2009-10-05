local({
## Prepare
cor.graph <- function(x) {
	panel.cor <- function(x, y, digits=3, cex.cor, use="pairwise.complete.obs", method="pearson", scale=TRUE) {
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
## Compute
## Print result
data <- data.frame (rock)

rk.header ("Correlation Matrix Plot", parameters=list ("Method", "pearson", "Exclusion", "pairwise.complete.obs", "Precision", "3 digits", "Scale text", "TRUE"))

rk.graph.on ()
try ({
	cor.graph (data)
})
rk.graph.off ()

rk.print("Legend:\t'***': p &lt; 0.001 -- '**': p &lt; 0.01 -- '*': p &lt; 0.05 -- '.'': p &lt; 0.1")
})
.rk.rerun.plugin.link(plugin="rkward::cor_graph", settings="digits.real=3.00\nmethod.string=pearson\nscale.state=TRUE\nuse.string=pairwise.complete.obs\nx.available=rock", label="Run again")
.rk.make.hr()
