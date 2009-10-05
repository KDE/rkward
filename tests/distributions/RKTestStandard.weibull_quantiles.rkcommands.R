local({
## Prepare
## Compute
result <- (qweibull (p = c (0.95), shape = 1.00, scale = 1.00, lower.tail=TRUE, log.p = FALSE))
## Print result
rk.header ("Weibull quantile", list ("Vector of probabilities", "c (0.95)", "Shape", "1.00", "Scale", "1.00", "Tail", "lower.tail=TRUE", "Probabilities p are given as", "log.p = FALSE"))
rk.results (result, titles="Weibull quantiles")
})
.rk.rerun.plugin.link(plugin="rkward::weibull_quantiles", settings="logp.string=log.p = FALSE\np.text=0.95\nscale.real=1.00\nshape.real=1.00\ntail.string=lower.tail=TRUE", label="Run again")
.rk.make.hr()
