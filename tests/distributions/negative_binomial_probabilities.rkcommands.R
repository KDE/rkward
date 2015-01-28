local({
## Compute
q <- 0:15
p <- pnbinom (q, size=5, prob=0.5)
## Print result
rk.header ("Negative Binomial distribution", parameters=list("Target number of successful trials"="5",
	"Probability of success in each trial"="0.5",
	"Tail"="Lower tail: P[X ≤ x]"))
rk.results (data.frame ("Quantile"=q, "Probability"=p, check.names=FALSE))
})
