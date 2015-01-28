local({
## Prepare
require (FAdist)
## Compute
q <- seq (-2, 5, length.out=20)
p <- pgumbel (q, location=0.0, scale=1.0)
## Print result
rk.header ("Gumbel distribution", parameters=list("Location"="0.0",
	"Scale"="1.0",
	"Tail"="Lower tail: P[X ≤ x]"))
rk.results (data.frame ("Quantile"=q, "Probability"=p, check.names=FALSE))
})
