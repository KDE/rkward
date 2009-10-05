local({
## Prepare
## Compute
result <- log(qweibull (p = c (0.95), shape = 1.00, scale = 1.00, lower.tail=FALSE, log.p = FALSE))
## Print result
rk.header ("Gumbel quantile", list ("Vector of probabilities", "c (0.95)", "Shape", "1.00", "Scale", "1.00", "Tail", "lower.tail=FALSE", "Probabilities p are given as", "log.p = FALSE"))
rk.results (result, titles="Gumbel quantiles")
})
.rk.rerun.plugin.link(plugin="rkward::gumbel_quantiles", settings="logp.string=log.p = FALSE\np.text=0.95\nscale.real=1.00\nshape.real=1.00\ntail.string=lower.tail=FALSE", label="Run again")
.rk.make.hr()
