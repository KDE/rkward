local({
## Print result
rk.header ("Beta distribution function", parameters=list("Number of Observations"="100",
	"Lower quantile"="0.00",
	"Upper quantile"="1.00",
	"Shape1 (a)"="2.00",
	"Shape2 (b)"="2.00",
	"Non-centrality parameter (ncp)"="0.00",
	"Tail"="Lower tail: P[X ≤ x]",
	"Logarithmic"="yes",
	"Function"="pbeta"))

rk.graph.on ()
try ({
	curve (pbeta(x, shape1=2.00, shape2=2.00, ncp=0.00, lower.tail = TRUE, log.p=TRUE), from=0.00, to=1.00, n=100)
})
rk.graph.off ()
})
