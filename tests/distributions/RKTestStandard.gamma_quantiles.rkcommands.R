local({
## Prepare
## Compute
result <- (qgamma (p = c (-5), shape = 1.00, rate = 1.00, lower.tail=TRUE, log.p = TRUE))

## Print result
rk.header ("Gamma quantile", list ("Vector of probabilities", "c (-5)", "Shape", "1.00", "Rate", "1.00", "Tail", "lower.tail=TRUE", "Probabilities p are given as", "log.p = TRUE"))
rk.results (result, titles="Gamma quantiles")
})
.rk.rerun.plugin.link(plugin="rkward::gamma_quantiles", settings="logp.string=log.p = TRUE\np.text=-5\nrate.real=1.00\nshape.real=1.00\ntail.string=lower.tail=TRUE", label="Run again")
.rk.make.hr()
