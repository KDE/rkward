local({
## Prepare
## Compute
result <- (pt (q = c (0.95), df = 1.00, ncp = 0.00, lower.tail=TRUE, log.p = FALSE))
## Print result
rk.header ("t probability", list ("Vector of quantiles", "c (0.95)", "Degrees of Freedom", "1.00", "non-centrality parameter", "0.00", "Tail", "lower.tail=TRUE", "Probabilities p are given as", "log.p = FALSE"));
rk.results (result, titles="t probabilities")
})
.rk.rerun.plugin.link(plugin="rkward::t_probabilities", settings="df.real=1.00\nlogp.string=log.p = FALSE\nncp.real=0.00\nq.text=0.95\ntail.string=lower.tail=TRUE", label="Run again")
.rk.make.hr()
