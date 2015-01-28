local({
## Compute
q <- 0:5
p <- ppois (q, lambda=1.0)
## Print result
rk.header ("Poisson distribution", parameters=list("λ (Lambda)"="1.0",
	"Tail"="Lower tail: P[X ≤ x]"))
rk.results (data.frame ("Quantile"=q, "Probability"=p, check.names=FALSE))
})
