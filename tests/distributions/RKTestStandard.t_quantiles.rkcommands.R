local({
## Prepare
## Compute
result <- (qt (p = c (0.95), df = 1.00, ncp = 0.00, lower.tail=TRUE, log.p = FALSE))
## Print result
rk.header ("t quantile", list ("Vector of probabilities", "c (0.95)", "Degrees of freedom", "1.00", "non-centrality parameter", "0.00", "Tail", "lower.tail=TRUE", "Probabilities p are given as", "log.p = FALSE"));
rk.results (result, titles="t quantiles")
})
.rk.rerun.plugin.link(plugin="rkward::t_quantiles", settings="df.real=1.00\nlogp.string=log.p = FALSE\nncp.real=0.00\np.text=0.95\ntail.string=lower.tail=TRUE", label="Run again")
.rk.make.hr()
