local({
## Compute
q <- seq.int (0, 25, by=2)
p <- pwilcox (q, m=5, n=5)
## Print result
rk.header ("Wilcoxon Rank Sum distribution", parameters=list("m (Numbers of observations in the first sample)"="5",
	"n (Numbers of observations in the second sample)"="5",
	"Tail"="Lower tail: P[X ≤ x]"))
rk.results (data.frame ("Quantile"=q, "Probability"=p, check.names=FALSE))
})
