local({
## Print result
rk.header ("Chisquare density function", list ("Number of Observations", "100", "Lower quantile", "0.30", "Upper quantile", "24.10", "Degrees of freedom", "4.00", "Non-centrality parameter", "0.00", "Scale", "normal", "Function", "dchisq"));

rk.graph.on ()
try ({
	curve (dchisq(x, df=4.00, ncp=0.00), from=0.30, to=24.10, n=100)
})
rk.graph.off ()
})
