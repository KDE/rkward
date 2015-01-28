local({
## Compute
q <- 0:10
p <- pgeom (q, prob=0.50)
## Print result
rk.header ("Geometric distribution", parameters=list("probability of success in each trial"="0.50",
	"Tail"="Lower tail: P[X ≤ x]"))
rk.results (data.frame ("Quantile"=q, "Probability"=p, check.names=FALSE))
})
