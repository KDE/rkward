local({
## Prepare
## Compute
result <- (qcauchy (p = c (0.95), location = -0.03000000, scale = 1.03000000, lower.tail=TRUE, log.p = FALSE))
## Print result
rk.header ("Cauchy quantiles", list ("Vector of probabilities", "c (0.95)", "Location", "-0.03000000", "Scale", "1.03000000", "Tail", "lower.tail=TRUE", "Probabilities p are given as", "log.p = FALSE"));
rk.results (result, titles="Cauchy quantiles")
})
.rk.rerun.plugin.link(plugin="rkward::cauchy_quantiles", settings="location.real=-0.03000000\nlogp.string=log.p = FALSE\np.text=0.95\nscale.real=1.03000000\ntail.string=lower.tail=TRUE", label="Run again")
.rk.make.hr()
local({
## Prepare
## Compute
result <- (qcauchy (p = c (-1, -2), location = 0.02000000, scale = 0.98000000, lower.tail=FALSE, log.p = TRUE))
## Print result
rk.header ("Cauchy quantiles", list ("Vector of probabilities", "c (-1, -2)", "Location", "0.02000000", "Scale", "0.98000000", "Tail", "lower.tail=FALSE", "Probabilities p are given as", "log.p = TRUE"));
rk.results (result, titles="Cauchy quantiles")
})
.rk.rerun.plugin.link(plugin="rkward::cauchy_quantiles", settings="location.real=0.02000000\nlogp.string=log.p = TRUE\np.text=-1 -2\nscale.real=0.98000000\ntail.string=lower.tail=FALSE", label="Run again")
.rk.make.hr()
