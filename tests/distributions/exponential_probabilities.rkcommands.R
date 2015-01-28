local({
## Compute
q <- seq (0, 5, length.out=20)
p <- pexp (q, rate=1.0)
## Print result
rk.header ("Exponential distribution", parameters=list("Rate"="1.0",
	"Tail"="Lower tail: P[X ≤ x]"))
rk.results (data.frame ("Quantile"=q, "Probability"=p, check.names=FALSE))
})
