local({
## Print result
rk.header ("Log-Normal distribution function", parameters=list("Number of Observations"="100",
	"Lower quantile"="0.01",
	"Upper quantile"="3.29",
	"Mean"="4.00",
	"Standard deviation"="1.00",
	"Tail"="Lower tail: P[X ≤ x]",
	"Logarithmic"="no",
	"Function"="plnorm"))

rk.graph.on ()
try ({
	curve (plnorm(x, meanlog=4.00, sdlog=1.00, lower.tail = TRUE), from=0.01, to=3.29, n=100)
})
rk.graph.off ()
})
