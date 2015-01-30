local({
## Print result
rk.header ("Poisson density function", parameters=list("Lower quantile"="0",
	"Upper quantile"="12",
	"Mean"="5.00",
	"Logarithmic"="no",
	"Function"="dpois"))

rk.graph.on ()
try ({
	curve (dpois(x, lambda=5.00), from=0, to=12, n=13, type="p")
})
rk.graph.off ()
})
