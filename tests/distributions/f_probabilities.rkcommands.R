local({
## Prepare
## Compute
result <- (pf (q = c (.1, .2), df1 = 1.02, df2 = 1.11,  ncp = 0.02, lower.tail=TRUE, log.p = FALSE))
## Print result
rk.header ("F probability", list ("Vector of quantiles", "c (.1, .2)", "Numerator degrees of freedom", "1.02", "Denominator degrees of freedom", "1.11", "non-centrality parameter", "0.02", "Tail", "lower.tail=TRUE", "Probabilities p are given as", "log.p = FALSE"));
rk.results (result, titles="F probabilities")
})
