local({
## Prepare
## Compute
result <- (pnbinom (q = c (0.95), size = 3, prob = 0.50, lower.tail=TRUE, log.p = TRUE))
## Print result
rk.header ("Negative Binomial probability", list ("Vector of quantiles", "c (0.95)", "Size", "3", "Probability of success in each trial", "0.50", "Tail", "lower.tail=TRUE", "Probabilities p are given as", "log.p = TRUE"))
rk.results (result, titles="Negative Binomial probabilities")
})
.rk.rerun.plugin.link(plugin="rkward::negative_binomial_probabilities", settings="logp.string=log.p = TRUE\nprob.real=0.50\nq.text=0.95\nsize.real=3.00\ntail.string=lower.tail=TRUE", label="Run again")
.rk.make.hr()
