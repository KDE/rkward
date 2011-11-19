local({
## Print result
rk.header ("Cauchy density function", list ("Number of Observations", "100", "Lower quantile", "-3.29", "Upper quantile", "3.29", "Location", "0.00", "Scale", "1.00", "Scale", "normal", "Function", "dcauchy"));

rk.graph.on ()
try ({
	curve (dcauchy(x, location=0.00, scale=1.00), from=-3.29, to=3.29, n=100)
})
rk.graph.off ()
})
