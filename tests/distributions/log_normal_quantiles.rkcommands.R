local({
## Compute
result <- (qlnorm (p = c (0.95), meanlog = 0.00, sdlog = 1.00, lower.tail=FALSE, log.p = FALSE))
## Print result
rk.header ("Log Normal quantile", list ("Vector of probabilities", "c (0.95)", "meanlog", "0.00", "sdlog", "1.00", "Tail", "lower.tail=FALSE", "Probabilities p are given as", "log.p = FALSE"))
rk.results (result, titles="Log Normal quantiles")
})
