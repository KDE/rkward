local({
## Compute
result <- (qnbinom (p = c (0.95), size = 4, prob = 0.50, lower.tail=TRUE, log.p = FALSE))
## Print result
rk.header ("Negative Binomial quantile", list ("Vector of probabilities", "c (0.95)", "Size", "4", "Probability of success in each trial", "0.50", "Tail", "lower.tail=TRUE", "Probabilities p are given as", "log.p = FALSE"))
rk.results (result, titles="Negative Binomial quantiles")
})
