local({
## Compute
result <- (qchisq (p = c (0.95), df = 1.00, ncp = 0.00, lower.tail=TRUE, log.p = FALSE))
## Print result
rk.header ("Chi-squared quantile", list ("Vector of probabilities", "c (0.95)", "Degrees of freedom", "1.00", "non-centrality parameter", "0.00", "Tail", "lower.tail=TRUE", "Probabilities p are given as", "log.p = FALSE"));
rk.results (result, titles="Chi-squared quantiles")
})
local({
## Compute
result <- (qchisq (p = c (-1, -2), df = 1.02, ncp = 3.00, lower.tail=FALSE, log.p = TRUE))
## Print result
rk.header ("Chi-squared quantile", list ("Vector of probabilities", "c (-1, -2)", "Degrees of freedom", "1.02", "non-centrality parameter", "3.00", "Tail", "lower.tail=FALSE", "Probabilities p are given as", "log.p = TRUE"));
rk.results (result, titles="Chi-squared quantiles")
})
