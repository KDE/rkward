local({
## Prepare
## Compute
result <- (qhyper (p = c (0.95), m = 1, n = 1, k = 1, lower.tail=TRUE, log.p = FALSE))
## Print result
rk.header ("Hypergeometric quantile", list ("Vector of probabilities", "c (0.95)", "Number of white balls in the urn", "1", "Number of black balls in the urn", "1", "Number of balls drawn from the urn", "1", "Tail", "lower.tail=TRUE", "Probabilities p are given as", "log.p = FALSE"))
rk.results (result, titles="Hypergeometric quantiles")
})
.rk.rerun.plugin.link(plugin="rkward::hypergeometric_quantiles", settings="k.real=1.00\nlogp.string=log.p = FALSE\nm.real=1.00\nn.real=1.00\np.text=0.95\ntail.string=lower.tail=TRUE", label="Run again")
.rk.make.hr()
