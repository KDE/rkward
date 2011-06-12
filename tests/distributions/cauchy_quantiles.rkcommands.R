local({
## Prepare
## Compute
result <- (qcauchy (p = c (0.95), location = -0.03, scale = 1.03, lower.tail=TRUE, log.p = FALSE))
## Print result
rk.header ("Cauchy quantiles", list ("Vector of probabilities", "c (0.95)", "Location", "-0.03", "Scale", "1.03", "Tail", "lower.tail=TRUE", "Probabilities p are given as", "log.p = FALSE"));
rk.results (result, titles="Cauchy quantiles")
})
local({
## Prepare
## Compute
result <- (qcauchy (p = c (-1, -2), location = 0.02, scale = 0.98, lower.tail=FALSE, log.p = TRUE))
## Print result
rk.header ("Cauchy quantiles", list ("Vector of probabilities", "c (-1, -2)", "Location", "0.02", "Scale", "0.98", "Tail", "lower.tail=FALSE", "Probabilities p are given as", "log.p = TRUE"));
rk.results (result, titles="Cauchy quantiles")
})
