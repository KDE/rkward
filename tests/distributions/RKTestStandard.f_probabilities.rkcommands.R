local({
## Prepare
## Compute
result <- (pf (q = c (.1, .2), df1 = 1.02000000, df2 = 1.11000000,  ncp = 0.02000000, lower.tail=TRUE, log.p = FALSE))
## Print result
rk.header ("F probability", list ("Vector of quantiles", "c (.1, .2)", "Numerator degrees of freedom", "1.02000000", "Denominator degrees of freedom", "1.11000000", "non-centrality parameter", "0.02000000", "Tail", "lower.tail=TRUE", "Probabilities p are given as", "log.p = FALSE"));
rk.results (result, titles="F probabilities")
})
.rk.rerun.plugin.link(plugin="rkward::f_probabilities", settings="df1.real=1.02000000\ndf2.real=1.11000000\nlogp.string=log.p = FALSE\nncp.real=0.02000000\nq.text=.1, .2\ntail.string=lower.tail=TRUE", label="Run again")
.rk.make.hr()
