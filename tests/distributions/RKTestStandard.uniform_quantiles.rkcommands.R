local({
## Prepare
## Compute
result <- (qunif (p = c (0.95), min = 0.00, max = 1.00, lower.tail=TRUE, log.p = FALSE))
## Print result
rk.header ("Uniform quantile", list ("Vector of probabilities", "c (0.95)", "Lower limits of the distribution", "0.00", "Upper limits of the distribution", "1.00", "Tail", "lower.tail=TRUE", "Probabilities p are given as", "log.p = FALSE"))
rk.results (result, titles="Uniform quantiles")
})
.rk.rerun.plugin.link(plugin="rkward::uniform_quantiles", settings="logp.string=log.p = FALSE\nmax.real=1.00\nmin.real=0.00\np.text=0.95\ntail.string=lower.tail=TRUE", label="Run again")
.rk.make.hr()
