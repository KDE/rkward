local({
## Compute
q <- seq (0, 5, length.out=20)
p <- pchisq (q, df=1.00, ncp=0.00)
## Print result
rk.header ("Chi-squared distribution", parameters=list("Degrees of Freedom"="1.00",
	"non-centrality parameter"="0.00",
	"Tail"="Lower tail: P[X ≤ x]"))
rk.results (data.frame ("Quantile"=q, "Probability"=p, check.names=FALSE))
})
