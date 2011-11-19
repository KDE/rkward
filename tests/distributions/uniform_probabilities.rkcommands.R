local({
## Compute
result <- (punif (q = c (0.95), min = 0.00, max = 1.00, lower.tail=TRUE, log.p = FALSE))
## Print result
rk.header ("Uniform probability", list ("Vector of quantiles", "c (0.95)", "Lower limits of the distribution", "0.00", "Upper limits of the distribution", "1.00", "Tail", "lower.tail=TRUE", "Probabilities p are given as", "log.p = FALSE"))
rk.results (result, titles="Uniform probabilities")
})
