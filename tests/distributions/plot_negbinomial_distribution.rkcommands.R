local({
## Prepare
## Compute
## Print result
rk.header ("Negative Binomial density function", list ("Lower quantile", "0", "Upper quantile", "24", "Target for number of successful trials", "12", "Probability of success in each trial", "0.75", "Scale", "normal", "Function", "dnbinom"));

rk.graph.on ()
try ({
	curve (dnbinom(x, size=12, prob=0.75), from=0, to=24, n=25, type="p")
})
rk.graph.off ()
})
