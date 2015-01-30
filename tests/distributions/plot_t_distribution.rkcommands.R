local({
## Print result
rk.header ("Student t distribution function", parameters=list("Number of Observations"="100",
	"Lower quantile"="-12.924",
	"Upper quantile"="12.924",
	"Degrees of freedom"="1.00",
	"Non-centrality parameter"="0.00",
	"Tail"="Lower tail: P[X ≤ x]",
	"Logarithmic"="no",
	"Function"="pt"))

rk.graph.on ()
try ({
	curve (pt(x, df=1.00, ncp=0.00, lower.tail = TRUE), from=-12.924, to=12.924, n=100)
})
rk.graph.off ()
})
