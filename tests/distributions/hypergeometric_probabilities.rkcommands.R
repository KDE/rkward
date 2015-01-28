local({
## Compute
q <- 0:10
p <- phyper (q, m=10, n=10, k=5)
## Print result
rk.header ("Hypergeometric distribution", parameters=list("m (Number of white balls in the urn)"="10",
	"n (Number of black balls in the urn)"="10",
	"k (Number of balls drawn from the urn)"="5",
	"Tail"="Lower tail: P[X ≤ x]"))
rk.results (data.frame ("Quantile"=q, "Probability"=p, check.names=FALSE))
})
