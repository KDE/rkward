local({
## Prepare
## Compute
## Print result
rk.header ("Geometric density function", list ("Lower quantile", "0", "Upper quantile", "12", "Probability of success on each trial", "0.50", "Scale", "normal", "Function", "dgeom"));

rk.graph.on ()
try ({
	curve (dgeom(x, prob=0.50), from=0, to=12, n=13, type="p")
})
rk.graph.off ()
})
