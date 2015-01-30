local({
## Print result
rk.header ("Hypergeometric distribution function", parameters=list("Lower quantile"="0",
	"Upper quantile (at most k)"="12",
	"Number of white balls (m)"="12",
	"Number of black balls (n)"="12",
	"Number of balls drawn (k)"="15",
	"Tail"="Lower tail: P[X ≤ x]",
	"Logarithmic"="no",
	"Function"="phyper"))

rk.graph.on ()
try ({
	curve (phyper(x, m=12, n=12, k=15, lower.tail = TRUE), from=0, to=12, n=13, type="p")
})
rk.graph.off ()
})
