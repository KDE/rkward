local({
## Prepare
## Compute
result <- (qlogis (p = c (0.95), location = 1.00, scale = 1.00, lower.tail=TRUE, log.p = FALSE))
## Print result
rk.header ("Logistic quantile", list ("Vector of probabilities", "c (0.95)", "Location", "1.00", "Scale", "1.00", "Tail", "lower.tail=TRUE", "Probabilities p are given as", "log.p = FALSE"))
rk.results (result, titles="Logistic quantiles")
})
.rk.rerun.plugin.link(plugin="rkward::logistic_quantiles", settings="location.real=1.00\nlogp.string=log.p = FALSE\np.text=0.95\nscale.real=1.00\ntail.string=lower.tail=TRUE", label="Run again")
.rk.make.hr()
