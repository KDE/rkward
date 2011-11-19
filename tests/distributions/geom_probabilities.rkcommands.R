local({
## Compute
result <- (pgeom (q = c (0.95), prob = 0.50, lower.tail=TRUE, log.p = FALSE))
## Print result
rk.header ("Geometric probability", list ("Vector of quantiles", "c (0.95)", "Probability of success in each trial", "0.50", "Tail", "lower.tail=TRUE", "Probabilities p are given as", "log.p = FALSE"))
rk.results (result, titles="Geometric probabilities")
})
