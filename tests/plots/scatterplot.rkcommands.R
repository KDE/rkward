local({
## Prepare
## Compute
Xvars <- list(women[["weight"]],swiss[["Education"]])
Yvars <- list(women[["height"]],swiss[["Catholic"]])

if (length(Xvars) != length(Yvars)) {
	stop("Unequal number of X and Y variables given")
}
# find range of X/Y values needed
Xrange <- range (c (Xvars), na.rm=TRUE)
Yrange <- range (c (Yvars), na.rm=TRUE)

type <- rep (c ('p'), length.out=length (Xvars));
col <- rep (c ('black', 'red'), length.out=length (Xvars));
cex <- rep (1, length.out=length (Xvars));
pch <- rep (1, length.out=length (Xvars));
## Print result
rk.header ("Scatterplot", parameters = list (
	"X variables"=paste (rk.get.description (women[["weight"]],swiss[["Education"]]), collapse=", "),
	"Y variables"=paste (rk.get.description (women[["height"]],swiss[["Catholic"]]), collapse=", ")))

rk.graph.on()

try ({
	# make frame and axes
	plot(Xrange, Yrange, type="n")
	
	# plot variables one X/Y pair at a time
	for (i in 1:length(Xvars)) {
		points (
			Xvars[[i]],
			Yvars[[i]],
			type = type[[i]],
			col = col[[i]],
			cex = cex[[i]],
			pch = pch[[i]]
		)
	}
})

rk.graph.off()
})
