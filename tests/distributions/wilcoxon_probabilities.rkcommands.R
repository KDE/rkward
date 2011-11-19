local({
## Compute
result <- (pwilcox (q = c (0.95), m = 2, n = 1, lower.tail=TRUE, log.p = FALSE))
## Print result
rk.header ("Wilcoxon Rank Sum probability", list ("Vector of quantiles", "c (0.95)", "m (Numbers of observations in the first sample)", "2", "n (Numbers of observations in the second sample)", "1", "Tail", "lower.tail=TRUE", "Probabilities p are given as", "log.p = FALSE"))
rk.results (result, titles="Wilcoxon Rank Sum probabilities")
})
