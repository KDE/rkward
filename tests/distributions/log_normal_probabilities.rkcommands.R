local({
## Prepare
## Compute
result <- (plnorm (q = c (0.95), meanlog = 0.00, sdlog = 1.00, lower.tail=TRUE, log.p = FALSE))
## Print result
rk.header ("Log Normal probability", list ("Vector of quantiles", "c (0.95)", "meanlog", "0.00", "sdlog", "1.00", "Tail", "lower.tail=TRUE", "Probabilities p are given as", "log.p = FALSE"))
rk.results (result, titles="Log Normal probabilities")
})
