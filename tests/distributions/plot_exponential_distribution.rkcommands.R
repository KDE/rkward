local({
## Print result
rk.header ("Exponential distribution function", list ("Number of Observations", "100", "Lower quantile", "0.00", "Upper quantile", "10.00", "Rate", "1.00", "Scale", "normal", "Tail","Upper", "Function", "pexp"));

rk.graph.on ()
try ({
	curve (pexp(x, rate=1.00, lower.tail = FALSE), from=0.00, to=10.00, n=100)
})
rk.graph.off ()
})
