local({
## Prepare
## Compute
result <- (qchisq (p = c (0.95), df = 1.00, ncp = 0.00, lower.tail=TRUE, log.p = FALSE))
## Print result
rk.header ("Chi-squared quantile", list ("Vector of probabilities", "c (0.95)", "Degrees of freedom", "1.00", "non-centrality parameter", "0.00", "Tail", "lower.tail=TRUE", "Probabilities p are given as", "log.p = FALSE"));
rk.results (result, titles="Chi-squared quantiles")
})
.rk.rerun.plugin.link(plugin="rkward::chi_squared_quantiles", settings="df.real=1.00\nlogp.string=log.p = FALSE\nncp.real=0.00\np.text=0.95\ntail.string=lower.tail=TRUE", label="Run again")
.rk.make.hr()
local({
## Prepare
## Compute
result <- (qchisq (p = c (-1, -2), df = 1.02, ncp = 3.00, lower.tail=FALSE, log.p = TRUE))
## Print result
rk.header ("Chi-squared quantile", list ("Vector of probabilities", "c (-1, -2)", "Degrees of freedom", "1.02", "non-centrality parameter", "3.00", "Tail", "lower.tail=FALSE", "Probabilities p are given as", "log.p = TRUE"));
rk.results (result, titles="Chi-squared quantiles")
})
.rk.rerun.plugin.link(plugin="rkward::chi_squared_quantiles", settings="df.real=1.02\nlogp.string=log.p = TRUE\nncp.real=3.00\np.text=-1 -2\ntail.string=lower.tail=FALSE", label="Run again")
.rk.make.hr()
