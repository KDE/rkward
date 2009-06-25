local({
## Prepare
## Compute
load (file="women.RData", envir=globalenv())
## Print result
rk.header("Load data", parameters=list("File", "women.RData",
	"Import to environment", "globalenv()"))
})
.rk.rerun.plugin.link(plugin="rkward::load_r_object", settings="file.selection=women.RData\nother_env.state=0", label="Run again")
.rk.make.hr()
