local({
## Prepare
## Compute
result <- (qlogis (p = c (0.95), location = 1.00, scale = 1.00, lower.tail=TRUE, log.p = FALSE))
## Print result
rk.header ("Logistic quantile", list ("Vector of probabilities", "c (0.95)", "Location", "1.00", "Scale", "1.00", "Tail", "lower.tail=TRUE", "Probabilities p are given as", "log.p = FALSE"))
rk.results (result, titles="Logistic quantiles")
})
