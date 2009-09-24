local({
## Prepare
## Compute
result <- (pexp (q = c (0.96), rate = 1.07000000, lower.tail=TRUE, log.p = FALSE))
## Print result
rk.header ("Exponential probabilities", list ("Vector of quantiles", "c (0.96)", "Rate", "1.07000000", "Tail", "lower.tail=TRUE", "Probabilities p are given as", "log.p = FALSE"))
rk.results (result, titles="Exponential probabilities")
})
.rk.rerun.plugin.link(plugin="rkward::exponential_probabilities", settings="logp.string=log.p = FALSE\nq.text=0.96\nrate.real=1.07000000\ntail.string=lower.tail=TRUE", label="Run again")
.rk.make.hr()
