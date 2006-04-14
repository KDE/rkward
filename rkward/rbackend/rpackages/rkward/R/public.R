# retrieve the rkward label (if any) of the given object
"rk.get.label" <- function (x) {
	if (is.call (x) || is.name (x)) {
		as.vector (attr (eval (x), ".rk.meta")[row.names (attr (eval (x), ".rk.meta")) == "label",1])
	} else {
		as.vector (attr (x, ".rk.meta")[row.names (attr (x, ".rk.meta")) == "label",1])
	}
}

# get a short name for the given object
"rk.get.short.name" <- function (x) {
	if (is.call (x) || is.name (x)) {
		.rk.make.short.name (deparse (x))
	} else {
		.rk.make.short.name (deparse (substitute (x)))
	}
}

# make a short name from the given arg (a character string)
".rk.make.short.name" <- function (x) {
	splt <- strsplit (x, "\"")[[1]]
	spltlen <- length (splt)
	if (spltlen == 1) {
		splt[1]
	} else {
		splt[spltlen - 1]
	}
}

# get descriptive strings for each of the arguments in ...
"rk.get.description" <- function (..., paste.sep=NULL) {
	args <- list(...)
	argnames <- rk.list.names (...)
	descript <- c ()

	for (i in 1:length (args)) {
		lbl <- rk.get.label (args[[i]])
		shortname <- .rk.make.short.name (argnames[i])

		if (is.null (lbl)) descript[i] <- shortname
		else descript[i] <- paste (shortname, " (", lbl, ")", sep="")
	}

	if (is.null (paste.sep)) {
		descript
	} else {
		paste (descript, collapse=paste.sep)
	}
}

# this is basically copied from R-base table (). Returns the arguments passed to ... as a character vector
"rk.list.names" <- function(..., deparse.level=2) {
	l <- as.list(substitute(list(...)))[-1]
	nm <- names(l)
	fixup <- if (is.null(nm)) 
		seq(along = l)
	else nm == ""
	dep <- sapply(l[fixup], function(x) switch(deparse.level + 1, "", if (is.symbol(x)) as.character(x) else "", deparse(x)[1]))
	if (is.null(nm)) 
		dep
	else {
		nm[fixup] <- dep
		nm
	}
}

"rk.sync" <- function (x) {
	object <- deparse (substitute (x))
	.rk.do.call ("sync", object)
}

"rk.get.tempfile.name" <- function (prefix="image", extension=".jpg") {
	return (.rk.do.call ("get.tempfile.name", c (prefix, extension)))
}

"rk.get.output.html.file" <- function () {
	return (.rk.do.call ("get.output.html.file", ""))
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
	filename <- rk.get.tempfile.name (prefix="graph", extension=".png");
	png (file.path (filename))
	cat (paste ("<img src=\"", filename,"\"><br>", sep=""),file=rk.get.output.html.file (), append=TRUE)
}

"rk.graph.off" <- function(){
	dev.off()
}

"rk.print" <- function(x,...) {
	htmlfile <- rk.get.output.html.file()
	if(require("R2HTML")==TRUE) {
		HTML(x, file=htmlfile,...)
	}
}

"rk.header" <- function (title, parameters=list ()) {
	cat (paste ("<h1>", title, "<h1>\n", sep=""))
	if (length (parameters)) {
		cat ("<h2>Parameters</h2>\n<ul>")
		len <- length (parameters)
		i <- 2
		while (i <= len) {
			cat (paste ("<li>", parameters[i-1], ": ", parameters[i], "</li>\n", sep=""))
			i <- i + 2
		}
		cat ("</ul>\n")
	}
	cat (date ())
	cat ("<br>\n")
}
