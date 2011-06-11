local({
## Prepare
## Compute
result <- (pbinom (q = c (0.95, 2), size = 3, prob = 0.50, lower.tail=TRUE, log.p = FALSE))
## Print result
rk.header ("Binomial tail probability", list ("Vector of quantiles", "c (0.95, 2)", "Binomial trials", "3", "Probability of success", "0.50", "Tail", "lower.tail=TRUE", "Probabilities p are given as", "log.p = FALSE"));
rk.results (result, titles="Binomial tail probabilities")
})
.rk.rerun.plugin.link(plugin="rkward::binomial_tail_probabilities", settings="logp.string=log.p = FALSE\nprob.real=0.50\nq.text=0.95 2\nsize.real=3.00\ntail.string=lower.tail=TRUE", label="Run again")
.rk.make.hr()
