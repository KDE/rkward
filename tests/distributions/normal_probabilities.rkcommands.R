local({
## Compute
q <- seq (-4, 4, length.out=20)
p <- pnorm (q, mean=0.00, sd=1.00)
## Print result
rk.header ("Normal distribution", parameters=list("mu (mean)"="0.00",
	"sigma (standard deviation)"="1.00",
	"Tail"="Lower tail: P[X ≤ x]"))
rk.results (data.frame ("Quantile"=q, "Probability"=p, check.names=FALSE))
})
