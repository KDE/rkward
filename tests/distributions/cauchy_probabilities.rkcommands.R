local({
## Compute
q <- seq (qcauchy (.01, location=0.00, scale=1.00), qcauchy (.99, location=0.00, scale=1.00), length.out=20)
p <- pcauchy (q, location=0.00, scale=1.00)
## Print result
rk.header ("Cauchy distribution", parameters=list("Location"="0.00",
	"Scale"="1.00",
	"Tail"="Lower tail: P[X ≤ x]"))
rk.results (data.frame ("Quantile"=q, "Probability"=p, check.names=FALSE))
})
