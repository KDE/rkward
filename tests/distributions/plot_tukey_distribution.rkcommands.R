local({
## Prepare
## Compute
## Print result
rk.header ("Tukey distribution function", list ("Number of Observations", "101", "Lower quantile", "-1.00", "Upper quantile", "8.00", "Sample size for range", "6", "Degreed of freedom for s", "5.00", "Number of groups", "1", "Scale", "normal", "Tail","Lower", "Function", "ptukey"));

rk.graph.on ()
try ({
	curve (ptukey(x, nmeans=6, df=5.00, nranges=1, lower.tail = TRUE), from=-1.00, to=8.00, n=101)
})
rk.graph.off ()
})
