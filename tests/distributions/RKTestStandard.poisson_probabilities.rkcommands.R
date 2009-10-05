local({
## Prepare
## Compute
result <- (ppois (q = c (0.95), lambda = 1.07, lower.tail=TRUE, log.p = TRUE))
## Print result
rk.header ("Poisson probability", list ("Vector of quantiles", "c (0.95)", "Lambda", "1.07", "Tail", "lower.tail=TRUE", "Probabilities p are given as", "log.p = TRUE"))
rk.results (result, titles="Poisson probabilities")
})
.rk.rerun.plugin.link(plugin="rkward::poisson_probabilities", settings="lambda.real=1.07\nlogp.string=log.p = TRUE\nq.text=0.95\ntail.string=lower.tail=TRUE", label="Run again")
.rk.make.hr()
