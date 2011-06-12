local({
## Prepare
## Compute
## Print result
rk.header ("F density function", list ("Number of Observations", "100", "Lower quantile", "0.001", "Upper quantile", "25.00", "Numerator degrees of freedom", "5.00", "Denominator degrees of freedom", "5.00", "Non-centrality", "0", "Scale", "logarithmic", "Function", "df"));

rk.graph.on ()
try ({
	curve (df(x, df1=5.00, df2=5.00, ncp=0, log=TRUE), from=0.001, to=25.00, n=100)
})
rk.graph.off ()
})
