local({
## Prepare
## Compute
result <- (pcauchy (q = c (0.95), location = 0.03000000, scale = 1.02000000, lower.tail=TRUE, log.p = FALSE))
## Print result
rk.header ("Cauchy probabilities", list ("Vector of quantiles", "c (0.95)", "Location", "0.03000000", "Scale", "1.02000000", "Tail", "lower.tail=TRUE", "Probabilities p are given as", "log.p = FALSE"))
rk.results (result, titles="Cauchy probabilities")
})
.rk.rerun.plugin.link(plugin="rkward::cauchy_probabilities", settings="location.real=0.03000000\nlogp.string=log.p = FALSE\nq.text=0.95\nscale.real=1.02000000\ntail.string=lower.tail=TRUE", label="Run again")
.rk.make.hr()
local({
## Prepare
## Compute
result <- (pcauchy (q = c (0.95), location = -0.02000000, scale = 1.03000000, lower.tail=FALSE, log.p = TRUE))
## Print result
rk.header ("Cauchy probabilities", list ("Vector of quantiles", "c (0.95)", "Location", "-0.02000000", "Scale", "1.03000000", "Tail", "lower.tail=FALSE", "Probabilities p are given as", "log.p = TRUE"))
rk.results (result, titles="Cauchy probabilities")
})
.rk.rerun.plugin.link(plugin="rkward::cauchy_probabilities", settings="location.real=-0.02000000\nlogp.string=log.p = TRUE\nq.text=0.95\nscale.real=1.03000000\ntail.string=lower.tail=FALSE", label="Run again")
.rk.make.hr()
