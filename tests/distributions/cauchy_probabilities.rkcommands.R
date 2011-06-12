local({
## Prepare
## Compute
result <- (pcauchy (q = c (0.95), location = 0.03, scale = 1.02, lower.tail=TRUE, log.p = FALSE))
## Print result
rk.header ("Cauchy probabilities", list ("Vector of quantiles", "c (0.95)", "Location", "0.03", "Scale", "1.02", "Tail", "lower.tail=TRUE", "Probabilities p are given as", "log.p = FALSE"))
rk.results (result, titles="Cauchy probabilities")
})
local({
## Prepare
## Compute
result <- (pcauchy (q = c (0.95), location = -0.02, scale = 1.03, lower.tail=FALSE, log.p = TRUE))
## Print result
rk.header ("Cauchy probabilities", list ("Vector of quantiles", "c (0.95)", "Location", "-0.02", "Scale", "1.03", "Tail", "lower.tail=FALSE", "Probabilities p are given as", "log.p = TRUE"))
rk.results (result, titles="Cauchy probabilities")
})
