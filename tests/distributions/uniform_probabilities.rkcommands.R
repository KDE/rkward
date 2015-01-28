local({
## Compute
q <- seq (0, 1, length.out=20)
p <- punif (q, min=0.00, max=1.00)
## Print result
rk.header ("Uniform distribution", parameters=list("Lower limit of the distribution"="0.00",
	"Upper limit of the distribution"="1.00",
	"Tail"="Lower tail: P[X ≤ x]"))
rk.results (data.frame ("Quantile"=q, "Probability"=p, check.names=FALSE))
})
