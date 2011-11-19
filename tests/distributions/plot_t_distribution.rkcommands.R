local({
## Print result
rk.header ("Student t distribution function", list ("Number of Observations", "100", "Minimum", "-12.924", "Maximum", "12.924", "Degrees of freedom", "1.00", "Non-centrality", "0.00", "Scale", "normal", "Tail","Lower", "Function", "pt"));

rk.graph.on ()
try ({
	curve (pt(x, df=1.00, ncp=0.00, lower.tail = TRUE), from=-12.924, to=12.924, n=100)
})
rk.graph.off ()
})
