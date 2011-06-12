local({
## Prepare
## Compute
result <- (pgamma (q = c (0.95), shape = 1.00, rate = 1.00, lower.tail=TRUE, log.p = FALSE))
## Print result
rk.header ("Gamma probability", list ("Vector of quantiles", "c (0.95)", "Shape", "1.00", "Rate", "1.00", "Tail", "lower.tail=TRUE", "Probabilities p are given as", "log.p = FALSE"))
rk.results (result, titles="Gamma probabilities")
})
