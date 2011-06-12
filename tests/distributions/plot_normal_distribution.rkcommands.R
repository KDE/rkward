local({
## Prepare
## Compute
## Print result
rk.header ("Normal density function", list ("Number of Observations", "100", "Lower quantile", "-3.29", "Upper quantile", "3.29", "Mean", "0.00", "Standard Deviation", "1.00", "Scale", "normal", "Function", "dnorm"));

rk.graph.on ()
try ({
	curve (dnorm(x, mean=0.00, sd=1.00), from=-3.29, to=3.29, n=100)
})
rk.graph.off ()
})
