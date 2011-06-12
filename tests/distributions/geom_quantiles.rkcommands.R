local({
## Prepare
## Compute
result <- (qgeom (p = c (0.95), prob = 0.50, lower.tail=TRUE, log.p = FALSE))
## Print result
rk.header ("Geometric quantile", list ("Vector of probabilities", "c (0.95)", "Probability of success in each trial", "0.50", "Tail", "lower.tail=TRUE", "Probabilities p are given as", "log.p = FALSE"))
rk.results (result, titles="Geometric quantiles")
})
