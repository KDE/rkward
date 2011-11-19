local({
## Compute
result <- (qnorm (p = c (0.95), mean = 0.00, sd = 1.00, lower.tail=TRUE, log.p = FALSE))
## Print result
rk.header ("Normal quantile", list ("Vector of probabilities", "c (0.95)", "mu", "0.00", "sigma", "1.00", "Tail", "lower.tail=TRUE", "Probabilities p are given as", "log.p = FALSE"));
rk.results (result, titles="Normal quantiles")
})
