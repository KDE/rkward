local({
## Prepare
## Compute
result <- (pt (q = c (0.95), df = 1.00, ncp = 0.00, lower.tail=TRUE, log.p = FALSE))
## Print result
rk.header ("t probability", list ("Vector of quantiles", "c (0.95)", "Degrees of Freedom", "1.00", "non-centrality parameter", "0.00", "Tail", "lower.tail=TRUE", "Probabilities p are given as", "log.p = FALSE"));
rk.results (result, titles="t probabilities")
})
