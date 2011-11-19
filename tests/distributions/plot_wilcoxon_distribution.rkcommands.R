local({
## Print result
rk.header ("Binomial density function", list ("Lower quantile", "0", "Upper quantile", "24", "First sample size", "4", "Second sample size", "6", "Scale", "normal", "Function", "dwilcox"));

rk.graph.on ()
try ({
	curve (dwilcox(x, m=4, n=6), from=0, to=24, n=25, type="p")
})
rk.graph.off ()
})
