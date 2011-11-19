local({
## Prepare
yrange <- range (swiss[["Catholic"]], na.rm=TRUE)
data.mean <- mean (swiss[["Catholic"]], na.rm=TRUE)
data.sd <- sd (swiss[["Catholic"]], na.rm=TRUE)
## Print result
rk.header ("Empirical Cumulative Distribution Function", list ("Variable", rk.get.description (swiss[["Catholic"]]), "Minimum", yrange[1], "Maximum", yrange[2]))

rk.graph.on ()
try ({
	plot.ecdf (swiss[["Catholic"]], , verticals=FALSE)
	curve (pnorm (x, mean=data.mean, sd=data.sd), from=yrange[1], to=yrange[2], add=TRUE, , col="blue")
	rug (swiss[["Catholic"]], 0.03, 0.50, side = 3)
})
rk.graph.off ()
})
