local({
## Prepare
## Compute
result <- (qf (p = c (0.95), df1 = 1.00, df2 = 1.00, ncp = 0.00, lower.tail=FALSE, log.p = FALSE))
## Print result
rk.header ("F quantile", list ("Vector of probabilities", "c (0.95)", "Numerator degrees of freedom", "1.00", "Denominator degrees of freedom", "1.00", "non-centrality parameter", "0.00", "Tail", "lower.tail=FALSE", "Probabilities p are given as", "log.p = FALSE"));
rk.results (result, titles="F quantiles")
})
.rk.rerun.plugin.link(plugin="rkward::f_quantiles", settings="df1.real=1.00\ndf2.real=1.00\nlogp.string=log.p = FALSE\nncp.real=0.00\np.text=0.95\ntail.string=lower.tail=FALSE", label="Run again")
.rk.make.hr()
