local({
## Print result
rk.header ("Histogram", list ("Variable"=rk.get.description (swiss[["Education"]]), "Break points"="Equally spaced vector of length 6",
	"Right closed"="yes",
	"Include in lowest cell"="yes",
	"Scale"="Density"))
rk.header ("Density curve", parameters=list("Bandwidth"="nrd",
	"Adjust Bandwidth"="4.00",
	"resolution"="512.00",
	"Remove Missing Values"="yes"), level=3)

rk.graph.on ()
try ({
	hist (swiss[["Education"]], breaks=(function(x) {y = extendrange(x,f=0.1); seq(from=y[1], to=y[2], length=6)})(swiss[["Education"]]), freq=FALSE, labels=TRUE, lty="solid", density=-1)
	lines(density(swiss[["Education"]], bw="nrd", adjust = 4.00, na.rm=TRUE, n = 512.00), col="blue")
})
rk.graph.off ()
})
