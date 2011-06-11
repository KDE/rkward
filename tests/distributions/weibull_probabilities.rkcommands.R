local({
## Prepare
## Compute
result <- (pweibull (q = c (0.95), shape = 1.00, scale = 1.00, lower.tail=TRUE, log.p = FALSE))
## Print result
rk.header ("Weibull probability", list ("Vector of quantiles", "c (0.95)", "Shape", "1.00", "Scale", "1.00", "Tail", "lower.tail=TRUE", "Probabilities p are given as", "log.p = FALSE"))
rk.results (result, titles="Weibull probabilities")
})
.rk.rerun.plugin.link(plugin="rkward::weibull_probabilities", settings="logp.string=log.p = FALSE\nq.text=0.95\nscale.real=1.00\nshape.real=1.00\ntail.string=lower.tail=TRUE", label="Run again")
.rk.make.hr()
