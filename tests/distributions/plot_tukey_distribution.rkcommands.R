local({
## Print result
rk.header ("Studentized Range (Tukey) distribution function", parameters=list("Number of Observations"="101",
	"Lower quantile"="-1.00",
	"Upper quantile"="8.00",
	"Number of observations"="6",
	"Degrees of freedom for standard deviation estimate"="5.0",
	"Tail"="Lower tail: P[X ≤ x]",
	"Logarithmic"="no",
	"Function"="ptukey"))

rk.graph.on ()
try ({
	curve (ptukey(x, nmeans=6, df=5.0, lower.tail = TRUE), from=-1.00, to=8.00, n=101)
})
rk.graph.off ()
})
