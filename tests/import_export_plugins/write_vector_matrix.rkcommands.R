local({
## Prepare
## Compute
write (x=testx, file="data", ncolumns=2, append=FALSE, sep=",")
## Print result
rk.header("Write Variables", parameters=list("File", "data",
	"Data", "testx"))
})
.rk.rerun.plugin.link(plugin="rkward::save_variables", settings="append.state=FALSE\ndata.available=testx\nfile.selection=data\nncolumns.real=2.00\nsep.string=,", label="Run again")
.rk.make.hr()
