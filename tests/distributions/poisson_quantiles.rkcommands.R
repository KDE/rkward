local({
## Prepare
## Compute
result <- (qpois (p = c (0.95), lambda = 1.00, lower.tail=TRUE, log.p = FALSE))
## Print result
rk.header ("Poisson quantile", list ("Vector of probabilities", "c (0.95)", "Lambda", "1.00", "Tail", "lower.tail=TRUE", "Probabilities p are given as", "log.p = FALSE"))
rk.results (result, titles="Poisson quantiles")
})
