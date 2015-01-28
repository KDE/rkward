local({
## Compute
q <- seq (0, qf (.99, df1=1.0, df2=10.0, ncp=0.0), length.out=20)
p <- pf (q, df1=1.0, df2=10.0, ncp=0.0)
## Print result
rk.header ("F distribution", parameters=list("Numerator degrees of Freedom"="1.0",
	"Denominator degrees of Freedom"="10.0",
	"non-centrality parameter"="0.0",
	"Tail"="Lower tail: P[X ≤ x]"))
rk.results (data.frame ("Quantile"=q, "Probability"=p, check.names=FALSE))
})
