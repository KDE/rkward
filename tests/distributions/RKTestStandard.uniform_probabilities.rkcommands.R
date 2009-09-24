local({
## Prepare
## Compute
result <- (punif (q = c (0.95), min = 0.00000000, max = 1.00000000, lower.tail=TRUE, log.p = FALSE))
## Print result
rk.header ("Uniform probability", list ("Vector of quantiles", "c (0.95)", "Lower limits of the distribution", "0.00000000", "Upper limits of the distribution", "1.00000000", "Tail", "lower.tail=TRUE", "Probabilities p are given as", "log.p = FALSE"))
rk.results (result, titles="Uniform probabilities")
})
.rk.rerun.plugin.link(plugin="rkward::uniform_probabilities", settings="logp.string=log.p = FALSE\nmax.real=1.00000000\nmin.real=0.00000000\nq.text=0.95\ntail.string=lower.tail=TRUE", label="Run again")
.rk.make.hr()
