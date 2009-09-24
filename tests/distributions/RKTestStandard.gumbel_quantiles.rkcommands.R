local({
## Prepare
## Compute
result <- log(qweibull (p = c (0.95), shape = 1.00000000, scale = 1.00000000, lower.tail=FALSE, log.p = FALSE))
## Print result
rk.header ("Gumbel quantile", list ("Vector of probabilities", "c (0.95)", "Shape", "1.00000000", "Scale", "1.00000000", "Tail", "lower.tail=FALSE", "Probabilities p are given as", "log.p = FALSE"))
rk.results (result, titles="Gumbel quantiles")
})
.rk.rerun.plugin.link(plugin="rkward::gumbel_quantiles", settings="logp.string=log.p = FALSE\np.text=0.95\nscale.real=1.00000000\nshape.real=1.00000000\ntail.string=lower.tail=FALSE", label="Run again")
.rk.make.hr()
