local({
## Print result
rk.header ("Weibull density function", parameters=list("Number of Observations"="100",
	"Lower quantile"="0.00",
	"Upper quantile"="5.00",
	"Shape"="2.00",
	"Scale"="1.00",
	"Logarithmic"="no",
	"Function"="dweibull"))

rk.graph.on ()
try ({
	curve (dweibull(x, shape=2.00, scale=1.00), from=0.00, to=5.00, n=100)
})
rk.graph.off ()
})
