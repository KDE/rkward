local({
## Prepare
## Compute
## Print result
rk.header ("Beta distribution function", list ("Number of Observations", "100", "Lower quantile", "0.00", "Upper quantile", "1.00", "Shape1", "2.00", "Shape2", "2.00", "Non-centrality parameter", "0.00", "Scale", "logarithmic", "Tail","Lower", "Function", "pbeta"));

rk.graph.on ()
try ({
	curve (pbeta(x, shape1=2.00, shape2=2.00, ncp=0.00, log.p=TRUE, lower.tail = TRUE), from=0.00, to=1.00, n=100)
})
rk.graph.off ()
})
