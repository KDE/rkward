local({
## Compute
q <- seq (0, qweibull (.99, shape=1.0, scale=1.0), length.out=20)
p <- pweibull (q, shape=1.0, scale=1.0)
## Print result
rk.header ("Weibull distribution", parameters=list("Shape"="1.0",
	"Scale"="1.0",
	"Tail"="Lower tail: P[X ≤ x]"))
rk.results (data.frame ("Quantile"=q, "Probability"=p, check.names=FALSE))
})
