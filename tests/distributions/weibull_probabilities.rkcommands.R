local({
## Compute
result <- (pweibull (q = c (0.95), shape = 1.00, scale = 1.00, lower.tail=TRUE, log.p = FALSE))
## Print result
rk.header ("Weibull probability", list ("Vector of quantiles", "c (0.95)", "Shape", "1.00", "Scale", "1.00", "Tail", "lower.tail=TRUE", "Probabilities p are given as", "log.p = FALSE"))
rk.results (result, titles="Weibull probabilities")
})
