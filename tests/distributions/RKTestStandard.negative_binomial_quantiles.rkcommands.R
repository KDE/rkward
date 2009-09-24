local({
## Prepare
## Compute
result <- (qnbinom (p = c (0.95), size = 4, prob = 0.50000000, lower.tail=TRUE, log.p = FALSE))
## Print result
rk.header ("Negative Binomial quantile", list ("Vector of probabilities", "c (0.95)", "Size", "4", "Probability of success in each trial", "0.50000000", "Tail", "lower.tail=TRUE", "Probabilities p are given as", "log.p = FALSE"))
rk.results (result, titles="Negative Binomial quantiles")
})
.rk.rerun.plugin.link(plugin="rkward::negative_binomial_quantiles", settings="logp.string=log.p = FALSE\np.text=0.95\nprob.real=0.50000000\nsize.real=4.000000\ntail.string=lower.tail=TRUE", label="Run again")
.rk.make.hr()
