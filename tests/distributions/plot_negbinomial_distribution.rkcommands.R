local({
## Print result
rk.header ("Negative Binomial density function", parameters=list("Lower quantile"="0",
	"Upper quantile"="24",
	"Target number of successful trials"="12",
	"Probability of success on each trial"="0.75",
	"Logarithmic"="no",
	"Function"="dnbinom"))

rk.graph.on ()
try ({
	curve (dnbinom(x, size=12, prob=0.75), from=0, to=24, n=25, type="p")
})
rk.graph.off ()
})
