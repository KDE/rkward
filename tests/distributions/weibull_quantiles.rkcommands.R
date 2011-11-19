local({
## Compute
result <- (qweibull (p = c (0.95), shape = 1.00, scale = 1.00, lower.tail=TRUE, log.p = FALSE))
## Print result
rk.header ("Weibull quantile", list ("Vector of probabilities", "c (0.95)", "Shape", "1.00", "Scale", "1.00", "Tail", "lower.tail=TRUE", "Probabilities p are given as", "log.p = FALSE"))
rk.results (result, titles="Weibull quantiles")
})
