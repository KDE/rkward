local({
## Prepare
## Compute
## Print result
rk.header ("Weibull density function", list ("Number of Observations", "100", "Lower quantile", "0.00", "Upper quantile", "5.00", "Shape", "2.00", "Scale", "1.00", "Scale", "normal", "Function", "dweibull"));

rk.graph.on ()
try ({
	curve (dweibull(x, shape=2.00, scale=1.00), from=0.00, to=5.00, n=100)
})
rk.graph.off ()
})
