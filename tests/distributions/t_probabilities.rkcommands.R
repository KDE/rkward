local({
## Compute
q <- seq (qt (.01, df=10.0, ncp=0.0), qt (.99, df=10.0, ncp=0.0), length.out=20)
p <- pt (q, df=10.0, ncp=0.0)
## Print result
rk.header ("t distribution", parameters=list("Degrees of Freedom"="10.0",
	"non-centrality parameter"="0.0",
	"Tail"="Lower tail: P[X ≤ x]"))
rk.results (data.frame ("Quantile"=q, "Probability"=p, check.names=FALSE))
})
