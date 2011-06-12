local({
## Prepare
## Compute
result <- (ppois (q = c (0.95), lambda = 1.07, lower.tail=TRUE, log.p = TRUE))
## Print result
rk.header ("Poisson probability", list ("Vector of quantiles", "c (0.95)", "Lambda", "1.07", "Tail", "lower.tail=TRUE", "Probabilities p are given as", "log.p = TRUE"))
rk.results (result, titles="Poisson probabilities")
})
