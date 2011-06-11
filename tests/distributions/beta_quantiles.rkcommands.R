local({
## Prepare
## Compute
result <- (qbeta (p = c (0.95), shape1 = 1.00, shape2 = 1.00, ncp = 0.00, lower.tail=TRUE, log.p = FALSE))
## Print result
rk.header ("Beta quantiles", list ("Vector of probabilities", "c (0.95)", "Shape 1", "1.00", "Shape 2", "1.00", "non-centrality parameter (ncp)", "0.00", "Tail", "lower.tail=TRUE", "Probabilities p are given as", "log.p = FALSE"));
rk.results (result, titles="Beta quantiles")
})
.rk.rerun.plugin.link(plugin="rkward::beta_quantiles", settings="logp.string=log.p = FALSE\nncp.real=0.00\np.text=0.95\nshape1.real=1.00\nshape2.real=1.00\ntail.string=lower.tail=TRUE", label="Run again")
.rk.make.hr()
local({
## Prepare
## Compute
result <- (qbeta (p = c (-1), shape1 = 1.04, shape2 = 1.03, ncp = 0.08, lower.tail=FALSE, log.p = TRUE))
## Print result
rk.header ("Beta quantiles", list ("Vector of probabilities", "c (-1)", "Shape 1", "1.04", "Shape 2", "1.03", "non-centrality parameter (ncp)", "0.08", "Tail", "lower.tail=FALSE", "Probabilities p are given as", "log.p = TRUE"));
rk.results (result, titles="Beta quantiles")
})
.rk.rerun.plugin.link(plugin="rkward::beta_quantiles", settings="logp.string=log.p = TRUE\nncp.real=0.08\np.text=-1\nshape1.real=1.04\nshape2.real=1.03\ntail.string=lower.tail=FALSE", label="Run again")
.rk.make.hr()
