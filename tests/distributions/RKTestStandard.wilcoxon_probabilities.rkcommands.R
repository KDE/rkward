local({
## Prepare
## Compute
result <- (pwilcox (q = c (0.95), m = 2, n = 1, lower.tail=TRUE, log.p = FALSE))
## Print result
rk.header ("Wilcoxon Rank Sum probability", list ("Vector of quantiles", "c (0.95)", "m (Numbers of observations in the first sample)", "2", "n (Numbers of observations in the second sample)", "1", "Tail", "lower.tail=TRUE", "Probabilities p are given as", "log.p = FALSE"))
rk.results (result, titles="Wilcoxon Rank Sum probabilities")
})
.rk.rerun.plugin.link(plugin="rkward::wilcoxon_probabilities", settings="logp.string=log.p = FALSE\nm.real=2.000000\nn.real=1.000000\nq.text=0.95\ntail.string=lower.tail=TRUE", label="Run again")
.rk.make.hr()
