"rk.get.label" <- function (x) {
	if (is.call (x) || is.name (x)) {
		as.vector (attr (eval (x), ".rk.meta")[row.names (attr (eval (x), ".rk.meta")) == "label",1])
	} else {
		as.vector (attr (x, ".rk.meta")[row.names (attr (x, ".rk.meta")) == "label",1])
	}
}

"rk.get.short.name" <- function (x) {
	if (is.call (x) || is.name (x)) {
		splt <- strsplit (deparse (x), "\"")
	} else {
		splt <- strsplit (deparse (substitute (x)), "\"")
	}
	spllen <- length (splt[[1]])
	if (spllen == 1) return (splt[[1]][1])
	return (splt[[1]][spllen-1])
}

"rk.get.description" <- function (x) {
	lbl <- rk.get.label (x)
	
	if (is.call (x) || is.name (x)) {
		splt <- strsplit (deparse (x), "\"")
	} else {
		splt <- strsplit (deparse (substitute (x)), "\"")
	}
	spllen <- length (splt[[1]])
	sn <- (splt[[1]][spllen-1])
	if (spllen == 1) sn <- splt[[1]][1]
	
	if (is.null (lbl)) return (sn)
	return (paste (sn, " (", lbl, ")", sep=""))
}

"rk.sync" <- function (x) {
	object <- deparse (substitute (x))
	.rk.do.call ("sync", object)
}

"rk.get.tempfile.name" <- function (prefix="image", extension=".jpg") {
	return (.rk.do.call ("get.tempfile.name", c (prefix, extension)))
}

# renames a named object in a data.frame/list without changing it's position
# TODO: create a generic function instead, that can handle all kinds of renames
"rk.rename.in.container" <- function (x, old_name, new_name) {
	temp <- (names (x) == old_name)
	i = 1;
	for (val in temp) {
		if (val) {
			eval (substitute (names (x)[i] <<- new_name))
			return ()
		}
		i = i+1;
	}
	error ("Could not find column with given name")
}


# Requests a graph to be written.
"rk.graph.on" <- function(){
	i=1
	while(file.exists(file.path(.rk.output.path,paste("graph",i,".png",sep="")))) {
		i=i+1
	}
	png(file.path(.rk.output.path,paste("graph",i,".png",sep="")))
	cat(paste("<img src=\"", paste("graph",i,".png",sep=""),"\"><br>",sep=""),file=.rk.output.file,append=TRUE)
}

"rk.graph.off" <- function(){
	dev.off()
}

"rk.print" <- function(x) {
	if(require(R2HTML)==TRUE) {
		#.HTML.file = .rk.output.file
		HTML(x)
	}
}


