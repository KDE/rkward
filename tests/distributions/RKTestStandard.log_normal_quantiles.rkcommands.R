local({
## Prepare
## Compute
result <- (qlnorm (p = c (0.95), meanlog = 0.00000000, sdlog = 1.00000000, lower.tail=FALSE, log.p = FALSE))
## Print result
rk.header ("Log Normal quantile", list ("Vector of probabilities", "c (0.95)", "meanlog", "0.00000000", "sdlog", "1.00000000", "Tail", "lower.tail=FALSE", "Probabilities p are given as", "log.p = FALSE"))
rk.results (result, titles="Log Normal quantiles")
})
.rk.rerun.plugin.link(plugin="rkward::log_normal_quantiles", settings="logp.string=log.p = FALSE\nmeanlog.real=0.00000000\np.text=0.95\nsdlog.real=1.00000000\ntail.string=lower.tail=FALSE", label="Run again")
.rk.make.hr()
