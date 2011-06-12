local({
## Prepare
## Compute
result <- (pnorm (q = c (0.95), mean = 0.00, sd = 1.00, lower.tail=TRUE, log.p = FALSE))
## Print result
rk.header ("Normal probability", list ("Vector of quantiles", "c (0.95)", "mu", "0.00", "sigma", "1.00", "Tail", "lower.tail=TRUE", "Probabilities p are given as", "log.p = FALSE"));
rk.results (result, titles="Normal probabilities")
})
