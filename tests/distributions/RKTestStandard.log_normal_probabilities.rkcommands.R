local({
## Prepare
## Compute
result <- (plnorm (q = c (0.95), meanlog = 0.00000000, sdlog = 1.00000000, lower.tail=TRUE, log.p = FALSE))
## Print result
rk.header ("Log Normal probability", list ("Vector of quantiles", "c (0.95)", "meanlog", "0.00000000", "sdlog", "1.00000000", "Tail", "lower.tail=TRUE", "Probabilities p are given as", "log.p = FALSE"))
rk.results (result, titles="Log Normal probabilities")
})
.rk.rerun.plugin.link(plugin="rkward::log_normal_probabilities", settings="logp.string=log.p = FALSE\nmeanlog.real=0.00000000\nq.text=0.95\nsdlog.real=1.00000000\ntail.string=lower.tail=TRUE", label="Run again")
.rk.make.hr()
