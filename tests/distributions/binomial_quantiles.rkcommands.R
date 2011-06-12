local({
## Prepare
## Compute
result <- (qbinom (p = c (0.95, .5), size = 3, prob = 0.50, lower.tail=TRUE, log.p = FALSE))
## Print result
rk.header ("Binomial quantile", list ("Vector of quantiles probabilities", "c (0.95, .5)", "Binomial trials", "3", "Probability of success", "0.50", "Tail", "lower.tail=TRUE", "Probabilities p are given as", "log.p = FALSE"));
rk.results (result, titles="Binomial quantiles")
})
