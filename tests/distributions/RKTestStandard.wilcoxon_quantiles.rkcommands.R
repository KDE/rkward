local({
## Prepare
## Compute
result <- (qwilcox (p = c (0.95), m = 1, n = 2, lower.tail=TRUE, log.p = FALSE))
## Print result
rk.header ("Wilcoxon Rank Sum quantile", list ("Vector of probabilities", "c (0.95)", "m (Numbers of observations in the first sample)", "1", "n (Numbers of observations in the second sample)", "2", "Tail", "lower.tail=TRUE", "Probabilities p are given as", "log.p = FALSE"))
rk.results (result, titles="Wilcoxon Rank Sum quantiles")
})
.rk.rerun.plugin.link(plugin="rkward::wilcoxon_quantiles", settings="logp.string=log.p = FALSE\nm.real=1.000000\nn.real=2.000000\np.text=0.95\ntail.string=lower.tail=TRUE", label="Run again")
.rk.make.hr()
