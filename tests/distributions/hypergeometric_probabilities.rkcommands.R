local({
## Prepare
## Compute
result <- (phyper (q = c (0, 1, 2, 3, 4, 5), m = 5, n = 4, k = 4, lower.tail=FALSE, log.p = FALSE))
## Print result
rk.header ("Hypergeometric probability", list ("Vector of quantiles", "c (0, 1, 2, 3, 4, 5)", "Number of white balls in the urn", "5", "Number of black balls in the urn", "4", "Number of balls drawn from the urn", "4", "Tail", "lower.tail=FALSE", "Probabilities p are given as", "log.p = FALSE"))
rk.results (result, titles="Hypergeometric probabilities")
})
.rk.rerun.plugin.link(plugin="rkward::hypergeometric_probabilities", settings="k.real=4.00\nlogp.string=log.p = FALSE\nm.real=5.00\nn.real=4.00\nq.text=0 1 2 3 4 5\ntail.string=lower.tail=FALSE", label="Run again")
.rk.make.hr()
