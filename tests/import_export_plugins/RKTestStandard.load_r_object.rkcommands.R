local({
## Prepare
## Compute
load (file="women.RData", envir=globalenv())
## Print result
rk.header("Load data", parameters=list("File", "women.RData",
	"Import to environment", "globalenv()"))
})
.rk.rerun.plugin.link(plugin="rkward::load_r_object", settings="envir.active=0\nenvir.objectname=my.env\nenvir.parent=.GlobalEnv\nfile.selection=women.RData", label="Run again")
.rk.make.hr()
