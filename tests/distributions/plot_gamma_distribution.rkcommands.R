local({
## Print result
rk.header ("Gamma density function", parameters=list("Number of Observations"="100",
	"Lower quantile"="0.01",
	"Upper quantile"="4.60",
	"Shape"="1.61",
	"Rate (lambda) or 1/Scale"="0.87",
	"Logarithmic"="no",
	"Function"="dgamma"))

rk.graph.on ()
try ({
	curve (dgamma(x, shape=1.61, rate=0.87), from=0.01, to=4.60, n=100)
})
rk.graph.off ()
})
