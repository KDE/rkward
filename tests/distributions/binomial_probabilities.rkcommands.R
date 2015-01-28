local({
## Compute
q <- 0:10
p <- pbinom (q, size=10, prob=0.50)
## Print result
rk.header ("Binomial distribution", parameters=list("Binomial trials"="10",
	"Probability of success"="0.50",
	"Tail"="Lower tail: P[X ≤ x]"))
rk.results (data.frame ("Quantile"=q, "Probability"=p, check.names=FALSE))
})
