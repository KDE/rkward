local({
## Print result
rk.header ("Wilcoxon Rank Sum density function", parameters=list("Lower quantile"="0",
	"Upper quantile (at most m*n)"="24",
	"Number of Observations in first sample (m)"="4",
	"Number of Observations in second sample (n)"="6",
	"Logarithmic"="no",
	"Function"="dwilcox"))

rk.graph.on ()
try ({
	curve (dwilcox(x, m=4, n=6), from=0, to=24, n=25, type="p")
})
rk.graph.off ()
})
