local({
## Prepare
## Compute
result <- (qgeom (p = c (0.95), prob = 0.50, lower.tail=TRUE, log.p = FALSE))
## Print result
rk.header ("Geometric quantile", list ("Vector of probabilities", "c (0.95)", "Probability of success in each trial", "0.50", "Tail", "lower.tail=TRUE", "Probabilities p are given as", "log.p = FALSE"))
rk.results (result, titles="Geometric quantiles")
})
.rk.rerun.plugin.link(plugin="rkward::geom_quantiles", settings="logp.string=log.p = FALSE\np.text=0.95\nprob.real=0.50\ntail.string=lower.tail=TRUE", label="Run again")
.rk.make.hr()
