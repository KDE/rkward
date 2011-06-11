local({
## Prepare
## Compute
save (testx, file="x.RData", ascii=TRUE, compress=TRUE)
## Print result
rk.header("Save R objects", parameters=list("File", "x.RData",
	"Variables", "testx"))
})
.rk.rerun.plugin.link(plugin="rkward::save_r", settings="ascii.state=TRUE\ncompress.state=TRUE\ndata.available=testx\nfile.selection=x.RData", label="Run again")
.rk.make.hr()
local({
## Prepare
## Compute
save (testy, file="y.RData", ascii=TRUE, compress=TRUE)
## Print result
rk.header("Save R objects", parameters=list("File", "y.RData",
	"Variables", "testy"))
})
.rk.rerun.plugin.link(plugin="rkward::save_r", settings="ascii.state=TRUE\ncompress.state=TRUE\ndata.available=testy\nfile.selection=y.RData", label="Run again")
.rk.make.hr()
