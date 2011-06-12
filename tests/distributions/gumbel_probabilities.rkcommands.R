local({
## Prepare
## Compute
result <- (pweibull (q = exp(c (0.95)), shape = 1.03, scale = 1.04, lower.tail=TRUE, log.p = FALSE))
## Print result
rk.header ("Gumbel probability", list ("Vector of quantiles", "c (0.95)", "Shape", "1.03", "Scale", "1.04", "Tail", "lower.tail=TRUE", "Probabilities p are given as", "log.p = FALSE"))
rk.results (result, titles="Gumbel probabilities")
})
