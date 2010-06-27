local({
## Prepare
require(ltm)
## Compute
unidim.res <- unidimTest(estimates.rasch)
## Print result
rk.header ("Unidimensionality check (estimates.rasch)")
rk.print (unidim.res)
# keep results in current workspace
.GlobalEnv$unidim.res <- unidim.res
})
.rk.rerun.plugin.link(plugin="rkward::ltm_unidimensional", settings="save_name.active=1\nsave_name.objectname=unidim.res\nsave_name.parent=.GlobalEnv\nspin_samples.real=100.00\nx.available=estimates.rasch", label="Run again")
.rk.make.hr()
