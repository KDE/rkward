local({
## Prepare
## Compute
result <- (qf (p = c (0.95), df1 = 1.00, df2 = 1.00, ncp = 0.00, lower.tail=FALSE, log.p = FALSE))
## Print result
rk.header ("F quantile", list ("Vector of probabilities", "c (0.95)", "Numerator degrees of freedom", "1.00", "Denominator degrees of freedom", "1.00", "non-centrality parameter", "0.00", "Tail", "lower.tail=FALSE", "Probabilities p are given as", "log.p = FALSE"));
rk.results (result, titles="F quantiles")
})
