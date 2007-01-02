# retrieve the rkward label (if any) of the given object
"rk.get.label" <- function (x) {
	if (is.call (x) || is.name (x)) {
		as.vector (attr (eval (x), ".rk.meta")[names (attr (eval (x), ".rk.meta")) == "label"])
	} else {
		as.vector (attr (x, ".rk.meta")[names (attr (x, ".rk.meta")) == "label"])
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
"rk.get.description" <- function (..., paste.sep=NULL, is.substitute=FALSE) {
	args <- list(...)
	if (is.substitute) {
		argnames <- list ()
		j <- 1
		for (symb in list (...)) {
			argnames[j] <- deparse (symb)
			j <- j + 1
		}
	} else {
		argnames <- rk.list.names (...)
	}
	descript <- c ()

	for (i in 1:length (args)) {
		lbl <- rk.get.label (args[[i]])
		if (is.substitute) {
			shortname <- .rk.make.short.name (as.character (argnames[i]))
		} else {
			shortname <- .rk.make.short.name (argnames[i])
		}

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

"rk.edit" <- function (x) {
	object <- deparse (substitute (x))
	.rk.do.call ("edit", object)
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
"rk.graph.on" <- function (width=480, height=480, ...)
{
    filename <- rk.get.tempfile.name(prefix = "graph", extension = ".png")
    png(filename=file.path(filename), width=width, height=height, ...)
    cat(paste("<img src=\"", filename, "\" width=\"", width, "\" height=\"", height, "\"><br>", sep = ""), 
        file = rk.get.output.html.file(), append = TRUE)
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

"rk.results" <- function (x, titles=NULL) {
	if (is.list (x)) {	# or a data.frame
		if (is.null (titles)) {
			titles <- names (x)
		}

		if (is.data.frame (x)) {
			cat ("<table border=\"1\">\n<tr>")
			for (i in 1:length (x)) {
				cat ("<td>", titles[i], "</td>", sep="")
			}
			cat ("</tr>\n")
			for (row in 1:dim (x)[1]) {
				cat ("<tr>")
				for (col in 1:dim (x)[2]) {
					cat ("<td>", x[row, col], "</td>", sep="")
				}
				cat ("</tr>\n")
			}
			cat ("</table>\n")
		} else {
			stop ("uninmplemented")
			# TODO: handling for regular lists. 
			# Should probably output a <ul></ul>
		}
	} else {
		stop ("uninmplemented")
		# TODO: handling for vectors. 
		# Should probably output a <ul></ul>
	}
}

"rk.make.repos.string" <- function () {
	x <- options ("repos")$repos
	len <- length (x)
	ret <- sprintf ("c (")
	first <- TRUE
	for (i in 1:len) {
		if (first) {
			first <- FALSE
		} else {
			ret <- sprintf ("%s, ", ret)
		}
		if (names (x)[i] != "") {
			ret <- sprintf ("%s%s=\"%s\"", ret, names (x)[i], x[i])
		} else {
			ret <- sprintf ("%s\"%s\"", ret, x[i])
		}
	}
	ret <- sprintf ("%s)", ret)
	ret
}

# utility to convert the rkward meta data of objects created in rkward < 0.4.0
# keep for a few months after 0.4.0 is out
"rk.convert.pre040" <- function (x) {
	if (missing (x)) {
		x <- list ()
		lst <- ls (all.names=TRUE, envir=globalenv ())
		for (childname in lst) {
	        	assign (childname, rk.convert.pre040 (get (childname, envir = globalenv ())), envir=globalenv ())
		}
		return (invisible ())
	}

	if (is.list (x)) {
		oa <- attributes (x)
		x <- lapply (x, function (X) rk.convert.pre040 (X))
		attributes (x) <- oa
	}

	a <- attr (x, ".rk.meta")
	if (is.null (a)) return (x)
	if (!is.data.frame (a)) return (x)
	if (length (a) < 1) return (x)
	an <- as.character (a[[1]])
	names (an) <- row.names (a)
	attr (x, ".rk.meta") <- an

	x
}
