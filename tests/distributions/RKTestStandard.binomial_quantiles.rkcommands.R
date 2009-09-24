local({
## Prepare
## Compute
result <- (qbinom (p = c (0.95, .5), size = 3, prob = 0.50000000, lower.tail=TRUE, log.p = FALSE))
## Print result
rk.header ("Binomial quantile", list ("Vector of quantiles probabilities", "c (0.95, .5)", "Binomial trials", "3", "Probability of success", "0.50000000", "Tail", "lower.tail=TRUE", "Probabilities p are given as", "log.p = FALSE"));
rk.results (result, titles="Binomial quantiles")
})
.rk.rerun.plugin.link(plugin="rkward::binomial_quantiles", settings="logp.string=log.p = FALSE\np.text=0.95 .5\nprob.real=0.50000000\nsize.real=3.000000\ntail.string=lower.tail=TRUE", label="Run again")
.rk.make.hr()
