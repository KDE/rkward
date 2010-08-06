local({
## Prepare
require(ltm)
## Compute
unidim.res <- unidimTest(estimates.rasch)
## Print result
rk.header ("Unidimensionality check (estimates.rasch)")
rk.print ("Call:")
rk.print.literal (deparse(unidim.res$call, width.cutoff=500))
rk.header ("Matrix of tertachoric correlations:", level=4)
rk.print (unidim.res$Rho)
rk.header ("Unidimensionality Check using Modified Parallel Analysis:", level=4)
rk.print ("Alternative hypothesis: <em>The second eigenvalue of the observed data is substantially larger than the second eigenvalue of data under the assumed IRT model</em>")
rk.print (paste("Second eigenvalue in the observed data:", round(unidim.res$Tobs[2], digits=3)))
rk.print (paste("Average of second eigenvalues in Monte Carlo samples:", round(mean(unidim.res$T.boot[,2]), digits=3)))
rk.print (paste("Monte Carlo samples:", dim(unidim.res$T.boot)[1]))
rk.print (paste("p-value:", round(unidim.res$p.value, digits=3)))
# keep results in current workspace
.GlobalEnv$unidim.res <- unidim.res
})
.rk.rerun.plugin.link(plugin="rkward::ltm_unidimensional", settings="save_name.active=1\nsave_name.objectname=unidim.res\nsave_name.parent=.GlobalEnv\nspin_samples.real=100.00\nx.available=estimates.rasch", label="Run again")
.rk.make.hr()
