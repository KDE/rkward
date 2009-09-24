local({
## Prepare
## Compute
result <- (pweibull (q = exp(c (0.95)), shape = 1.03000000, scale = 1.04000000, lower.tail=TRUE, log.p = FALSE))
## Print result
rk.header ("Gumbel probability", list ("Vector of quantiles", "c (0.95)", "Shape", "1.03000000", "Scale", "1.04000000", "Tail", "lower.tail=TRUE", "Probabilities p are given as", "log.p = FALSE"))
rk.results (result, titles="Gumbel probabilities")
})
.rk.rerun.plugin.link(plugin="rkward::gumbel_probabilities", settings="logp.string=log.p = FALSE\nq.text=0.95\nscale.real=1.04000000\nshape.real=1.03000000\ntail.string=lower.tail=TRUE", label="Run again")
.rk.make.hr()
