local({
## Compute
q <- seq (0, qgamma (.99, shape=1.0, rate=1.0), length.out=20)
p <- pgamma (q, shape=1.0, rate=1.0)
## Print result
rk.header ("Gamma distribution", parameters=list("Shape"="1.0",
	"Rate"="1.0",
	"Tail"="Lower tail: P[X ≤ x]"))
rk.results (data.frame ("Quantile"=q, "Probability"=p, check.names=FALSE))
})
