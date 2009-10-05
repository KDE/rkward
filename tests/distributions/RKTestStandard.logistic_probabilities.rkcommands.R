local({
## Prepare
## Compute
result <- (plogis (q = c (0.95), location = 1.04, scale = 1.00, lower.tail=TRUE, log.p = TRUE))
## Print result
rk.header ("Logistic probability", list ("Vector of quantiles", "c (0.95)", "Location", "1.04", "Scale", "1.00", "Tail", "lower.tail=TRUE", "Probabilities p are given as", "log.p = TRUE"))
rk.results (result, titles="Logistic probabilities")
})
.rk.rerun.plugin.link(plugin="rkward::logistic_probabilities", settings="location.real=1.04\nlogp.string=log.p = TRUE\nq.text=0.95\nscale.real=1.00\ntail.string=lower.tail=TRUE", label="Run again")
.rk.make.hr()
