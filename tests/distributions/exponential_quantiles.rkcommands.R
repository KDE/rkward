local({
## Compute
result <- (qexp (p = c (-1.1), rate = 1.05, lower.tail=FALSE, log.p = TRUE))
## Print result
rk.header ("Exponential quantiles", list ("Vector of probabilities", "c (-1.1)", "Rate", "1.05", "Tail", "lower.tail=FALSE", "Probabilities p are given as", "log.p = TRUE"))
rk.results (result, titles="Exponential quantiles")
})
