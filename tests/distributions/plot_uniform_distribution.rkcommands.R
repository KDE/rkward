local({
## Prepare
## Compute
## Print result
rk.header ("Uniform density function", list ("Number of Observations", "100", "Lower quantile", "-1.00", "Upper quantile", "2.00", "Minimum", "0.00", "Maximum", "1.00", "Scale", "normal", "Function", "dunif"));

rk.graph.on ()
try ({
	curve (dunif(x, min=0.00, max=1.00), from=-1.00, to=2.00, n=100)
})
rk.graph.off ()
})
