local({
## Print result
rk.header ("Binomial density function", list ("Lower quantile", "0", "Upper quantile", "12", "Number of trials", "12", "Probability of success on each trial", "0.50", "Scale", "normal", "Function", "dbinom"));

rk.graph.on ()
try ({
	curve (dbinom(x, size=12, prob=0.50), from=0, to=12, n=13, type="p")
})
rk.graph.off ()
})
