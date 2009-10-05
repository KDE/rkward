local({
## Prepare
## Compute
result <- (pgamma (q = c (0.95), shape = 1.00, rate = 1.00, lower.tail=TRUE, log.p = FALSE))
## Print result
rk.header ("Gamma probability", list ("Vector of quantiles", "c (0.95)", "Shape", "1.00", "Rate", "1.00", "Tail", "lower.tail=TRUE", "Probabilities p are given as", "log.p = FALSE"))
rk.results (result, titles="Gamma probabilities")
})
.rk.rerun.plugin.link(plugin="rkward::gamma_probabilities", settings="logp.string=log.p = FALSE\nq.text=0.95\nrate.real=1.00\nshape.real=1.00\ntail.string=lower.tail=TRUE", label="Run again")
.rk.make.hr()
