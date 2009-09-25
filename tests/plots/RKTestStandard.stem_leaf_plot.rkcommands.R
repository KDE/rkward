local({
## Prepare
## Compute
## Print result
rk.header ("Stem-and-Leaf Plot",
	parameters=list ("Variable", paste (rk.get.description (swiss[["Fertility"]])), "Plot Length", "1.50000000","Plot Width", "80.00000000", "Tolerance", "0.00000001"))

rk.print.literal(capture.output(stem(swiss[["Fertility"]], scale = 1.50000000, width = 80.00000000, atom = 0.00000001)))
})
.rk.rerun.plugin.link(plugin="rkward::stem", settings="atom.real=0.00000001\nscale.real=1.50000000\nwidth.real=80.00000000\nx.available=swiss[[\\\"Fertility\\\"]]", label="Run again")
.rk.make.hr()
