local({
## Compute
q <- seq (0, qlnorm (.99, meanlog=0.0, sdlog=1.0), length.out=20)
p <- plnorm (q, meanlog=0.0, sdlog=1.0)
## Print result
rk.header ("Log Normal distribution", parameters=list("meanlog (mean on log scale)"="0.0",
	"sdlog (standard deviation on log scale)"="1.0",
	"Tail"="Lower tail: P[X ≤ x]"))
rk.results (data.frame ("Quantile"=q, "Probability"=p, check.names=FALSE))
})
