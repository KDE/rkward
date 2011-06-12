local({
## Prepare
## Compute
## Print result
rk.header ("Stem-and-Leaf Plot",
	parameters=list ("Variable", paste (rk.get.description (swiss[["Fertility"]])), "Plot Length", "1.50","Plot Width", "80.00", "Tolerance", "0.01"))

rk.print.literal(capture.output(stem(swiss[["Fertility"]], scale = 1.50, width = 80.00, atom = 0.01)))
})
