local({
## Prepare
## Compute
result <- (qpois (p = c (0.95), lambda = 1.00, lower.tail=TRUE, log.p = FALSE))
## Print result
rk.header ("Poisson quantile", list ("Vector of probabilities", "c (0.95)", "Lambda", "1.00", "Tail", "lower.tail=TRUE", "Probabilities p are given as", "log.p = FALSE"))
rk.results (result, titles="Poisson quantiles")
})
.rk.rerun.plugin.link(plugin="rkward::poisson_quantiles", settings="lambda.real=1.00\nlogp.string=log.p = FALSE\np.text=0.95\ntail.string=lower.tail=TRUE", label="Run again")
.rk.make.hr()
