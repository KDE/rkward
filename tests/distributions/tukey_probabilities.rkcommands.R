local({
## Compute
q <- seq (0, qtukey (.99, nmeans=5, df=5.0), length.out=20)
p <- ptukey (q, nmeans=5, df=5.0)
## Print result
rk.header ("Studentized Range (Tukey) distribution", parameters=list("Number of observations"="5",
	"Degrees of freedom for standard deviation estimate"="5.0",
	"Tail"="Lower tail: P[X ≤ x]"))
rk.results (data.frame ("Quantile"=q, "Probability"=p, check.names=FALSE))
})
