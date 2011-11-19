local({
## Print result
rk.header ("Logistic density function", list ("Number of Observations", "100", "Lower quantile", "-3.29", "Upper quantile", "3.29", "Location", "0.00", "Scale", "1.00", "Scale", "normal", "Function", "dlogis"));

rk.graph.on ()
try ({
	curve (dlogis(x, location=0.00, scale=1.00), from=-3.29, to=3.29, n=100)
})
rk.graph.off ()
})
