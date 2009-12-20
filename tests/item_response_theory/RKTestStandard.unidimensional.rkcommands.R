local({
## Prepare
require(ltm)
## Compute
unidim.res <- unidimTest(estimates.rasch)
## Print result
rk.header ("Unidimensionality check (estimates.rasch)")
rk.print (unidim.res)
# keep results in current workspace
unidim.res <<- unidim.res
})
.rk.rerun.plugin.link(plugin="rkward::ltm_unidimensional", settings="chk_save.state=save\nsave_name.selection=unidim.res\nspin_samples.real=100.00\nx.available=estimates.rasch", label="Run again")
.rk.make.hr()
