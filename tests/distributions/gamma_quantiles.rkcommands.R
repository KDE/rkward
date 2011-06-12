local({
## Prepare
## Compute
result <- (qgamma (p = c (-5), shape = 1.00, rate = 1.00, lower.tail=TRUE, log.p = TRUE))

## Print result
rk.header ("Gamma quantile", list ("Vector of probabilities", "c (-5)", "Shape", "1.00", "Rate", "1.00", "Tail", "lower.tail=TRUE", "Probabilities p are given as", "log.p = TRUE"))
rk.results (result, titles="Gamma quantiles")
})
