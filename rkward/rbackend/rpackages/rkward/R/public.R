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
	.rk.do.call (paste ("sync\t", object, "\n", sep=""))
}
