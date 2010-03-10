# retrieve the rkward label (if any) of the given object
"rk.get.label" <- function (x) {
	if (is.call (x) || is.name (x)) {
		ret <- attr (eval (x), ".rk.meta")[names (attr (eval (x), ".rk.meta")) == "label"]
	} else {
		ret <- attr (x, ".rk.meta")[names (attr (x, ".rk.meta")) == "label"]
	}
	as.character (as.vector (ret))
}

# set rkward label
"rk.set.label" <- function (x, label, envir=parent.frame()) {
	if (is.call (x) || is.name (x)) {
		meta <- attr (eval (x, envir=envir), ".rk.meta")
	} else {
		meta <- attr (x, ".rk.meta")
	}
	meta[["label"]] <- as.character (label)
	eval(substitute(attr(x, ".rk.meta") <- meta), envir = envir)
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
# e.g. return "b" for a[["b"]] (but 'a::"b"' for a::"b"
".rk.make.short.name" <- function (x) {
	splt <- strsplit (x, "[[\"", fixed=TRUE)[[1]]
	spltlen <- length (splt)
	if (spltlen == 1) {
		splt[1]
	} else {
		strsplit (splt[spltlen], "\"]]", fixed=TRUE)[[1]][1]
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

		if (is.null (lbl) || (length (lbl) < 1)) descript[i] <- shortname
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

# should this really be public?
"rk.sync" <- function (x) {
	object <- deparse (substitute (x))
	.rk.do.call ("sync", object)
}

# should this really be public?
"rk.sync.global" <- function () {
	.rk.do.call("syncglobal", ls (envir=globalenv (), all.names=TRUE))
}

"rk.edit" <- function (x) {
	object <- deparse (substitute (x))
	.rk.do.call ("edit", object)
}

"rk.get.tempfile.name" <- function (prefix="image", extension=".jpg") {
	return (.rk.do.call ("get.tempfile.name", c (prefix, extension)))
}

"rk.get.output.html.file" <- function () {
	return (.rk.output.html.file)
}

"rk.set.output.html.file" <- function (x) {
	stopifnot (is.character (x))
	assign (".rk.output.html.file", x, as.environment ("package:rkward"))

	if (!file.exists (x)) {
		.rk.cat.output (paste ("<?xml version=\"1.0\" encoding=\"", .Call ("rk.locale.name"), "\"?>\n", sep=""));
		.rk.cat.output ("<html><head><title>RKWard Output</title></head>\n<body>\n")
		# This initial output mostly to indicate the output is really there, just empty for now
		.rk.cat.output (paste ("<pre>RKWard output initialized on", date (), "</pre>\n"));
	}

	# needs to come after initialization, so initialization alone does not trigger an update during startup
	.rk.do.call ("set.output.file", x);
}

# renames a named object in a data.frame/list without changing it's position
# TODO: create a generic function instead, that can handle all kinds of renames
"rk.rename.in.container" <- function (x, old_name, new_name, envir=parent.frame()) {
	temp <- (names (x) == old_name)
	i = 1;
	for (val in temp) {
		if (val) {
			eval (substitute (names (x)[i] <- new_name), envir=envir)
			return ()
		}
		i = i+1;
	}
	error ("Could not find column with given name")
}

# Requests a graph to be written.
rk.graph.on <- function (device.type=getOption ("rk.graphics.type"), width=getOption ("rk.graphics.width"), height=getOption ("rk.graphics.height"), quality, ...) 
{
	if (!is.numeric (width)) width <- 480
	if (!is.numeric (height)) height <- 480
	if (is.null (device.type)) device.type <- "PNG"	# default behavior is PNG for now

	ret <- NULL
	if (device.type == "PNG") {
		filename <- rk.get.tempfile.name(prefix = "graph", extension = ".png")
		ret <- png(filename = file.path(filename), width = width, height = height, ...)
		.rk.cat.output(paste("<img src=\"", filename, "\" width=\"", width, 
			"\" height=\"", height, "\"><br>", sep = ""))
	} else if (device.type == "JPG") {
		if (missing (quality)) quality = getOption ("rk.graphics.jpg.quality", 75)
		filename <- rk.get.tempfile.name(prefix = "graph", extension = ".jpg")
		ret <- jpeg(filename = file.path(filename), width = width, height = height, "quality"=quality, ...)
		.rk.cat.output(paste("<img src=\"", filename, "\" width=\"", width, 
			"\" height=\"", height, "\"><br>", sep = ""))
	} else if (device.type == "SVG") {
		if (!capabilities ("cairo")) {	# cairo support is not always compiled in
			require (cairoDevice)
			svg <- Cairo_svg
		}
		filename <- rk.get.tempfile.name(prefix = "graph", extension = ".svg")
		ret <- svg(filename = file.path(filename), ...)
		.rk.cat.output(paste("<object data=\"", filename, "\" type=\"image/svg+xml\" width=\"", width, 
			"\" height=\"", height, "\">\n", sep = ""))
		.rk.cat.output(paste("<param name=\"src\" value=\"", filename, "\">\n", sep = ""))
		.rk.cat.output(paste("This browser appears incapable of displaying SVG object. The SVG source is at:", filename))
		.rk.cat.output("</object>")
	} else {
		stop (paste ("Device type \"", device.type, "\" is unknown to RKWard", sep=""))
	}

	invisible (ret)
}

"rk.graph.off" <- function(){
	.rk.cat.output ("\n")	# so the output will be auto-refreshed
	dev.off()
}

"rk.print" <- function(x,...) {
	htmlfile <- rk.get.output.html.file()
	if(require("R2HTML")==TRUE) {
		HTML(x, file=htmlfile,...)
	}
}

"rk.header" <- function (title, parameters=list (), level=1) {
	sink (rk.get.output.html.file(), append=TRUE)
	on.exit (sink ())

	cat ("<h", level, ">", title, "</h", level, ">\n", sep="")
	# legacy handling: parameter=value used to be passed as parameter, value
	if (!is.null (names (parameters))) {
		pnames <- names (parameters)
		p <- list ()
		for (i in 1:length (parameters)) {
			p[i*2-1] <- pnames[i]
			p[i*2] <- parameters[i]
		}
		parameters <- p
	}
	if (length (parameters)) {
		cat ("<h", level + 1, ">Parameters</h", level + 1, ">\n<ul>", sep="")
		len <- length (parameters)
		i <- 2
		while (i <= len) {
			cat ("<li>", parameters[[i-1]], ": ", parameters[[i]], "</li>\n", sep="")
			i <- i + 2
		}
		cat ("</ul>\n")
	}
	if (level==1) cat (date ())
	cat ("<br>\n")
}

"rk.results" <- function (x, titles=NULL) {
	sink (rk.get.output.html.file(), append=TRUE)
	on.exit (sink ())

	# convert 2d tables to data.frames with values labelled
	if (is.table(x) && (length(dim(x)) == 2)) {
		rows = dim(x)[1]
		cols = dim(x)[2]
		if (is.null(titles)) {
			titles <- names(dimnames(x))
		}
		rn <- c ()   # row names
		for (row in 1:rows) rn[row] <- paste (titles[1], "=", dimnames(x)[[1]][row])
		internal <- data.frame (cbind (x))
		temp <- data.frame (as.character (rn), stringsAsFactors=FALSE)
		for (col in 1:cols) temp[[col+1]] <- internal[[col]]
		titles <- c ("", paste (titles[2], "=", names (internal)))
		x <- temp
	}

	if (is.list (x)) {	# or a data.frame
		if (is.null (titles)) {
			titles <- names (x)
		}

		cat ("<table border=\"1\">\n<tr>")
		try ({	# if anything fails, make sure the "</table>" is still printed
			for (i in 1:length (x)) {
				cat ("<td>", titles[i], "</td>", sep="")
			}
			cat ("</tr>\n")
	
			if (is.data.frame (x)) {
				for (row in 1:dim (x)[1]) {
					cat ("<tr>")
					for (col in 1:dim (x)[2]) {
						cat ("<td>", x[row, col], "</td>", sep="")
					}
					cat ("</tr>\n")
				}
			} else {		# generic list
				cat ("<tr>")
				for (col in x) {
					col <- as.vector (col)
					cat ("<td>")
					for (row in 1:length (col)) {
						if (row != 1) cat ("\n<br/>")
						cat (col[row])
					}
					cat ("</td>")
				}
				cat ("</tr>\n")
			}
		})
		cat ("</table>\n")
	} else if (is.vector (x)) {
		cat ("<h3>", titles[1], ": ", sep="")
		cat (x)
		cat ("</h3>")
	} else {
		stop ("uninmplemented")
	}
}

"rk.print.literal" <- function (x) {
	cat ("<pre>", paste (x, collapse="\n"), "</pre>\n", sep="", file=rk.get.output.html.file(), append=TRUE);
}

# Describe the alternative (H1) of an htest.
# This code adapted from stats:::print.htest
"rk.describe.alternative" <- function (x) {
	res <- ""
	if (!is.null(x$alternative)) {
		if (!is.null(x$null.value)) {
 			if (length(x$null.value) == 1) {
 				alt.char <- switch(x$alternative, two.sided = "not equal to", less = "less than", greater = "greater than")
 				res <- paste ("true", names(x$null.value), "is", alt.char, x$null.value)
 			} else {
 				res <- paste (x$alternative, "\nnull values:\n", x$null.value)
 			}
		} else {
			res <-  (x$alternative)
		}
	}
	res
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
		if (!(is.null (names (x)) || (names (x)[i] == ""))) {
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

"rk.edit.files" <- function (file = file, title = file, name = NULL)
{
	if (!is.character (file)) {
		nfile = tempfile()
		env = environment (file)
		dput (file, file=nfile, control=c ("useSource", "keepNA", "keepInteger", "showAttributes"))
		.Call("rk.edit.files", nfile, title, name)
		x <- dget (nfile)
		environment (x) <- env
		return (x)
	}
	invisible (.Call ("rk.edit.files", file, title, name))
}

"rk.show.files" <- function (file = file, title = file, wtitle = NULL, delete=FALSE)
{
	invisible (.Call ("rk.show.files", as.character (file), as.character (title), as.character (wtitle), delete))
}

"rk.show.html" <- function (url) {
	invisible (.rk.do.call ("showHTML", as.character (url)));
}

"rk.call.plugin" <- function (plugin, ..., submit.mode = c ("manual", "auto", "submit")) {
	# prepare arguments
	settings <- list (...)
	callstrings <- list ()
	callstrings[1] <- plugin
	callstrings[2] <- match.arg (submit.mode)
	if (length (settings) > 0) {
		for (i in 1:length(settings)) {
			# properly passing on escaped characters is a pain. This seems to work.
			deparsed <- deparse (settings[[i]])
			deparsed_unquoted <- substr (deparsed, 2, nchar (deparsed) - 1)
			callstrings[i + 2] <- paste(names(settings)[i], deparsed_unquoted, 
			sep = "=")
		}
	}

	# do call
	res <- .rk.do.call ("doPlugin", callstrings)

	# handle result
	if (!is.null (res)) {
		if (res$type == "warning") {
			warning (res$message)
		} else {
			stop (res$message)
		}
	}

	invisible (TRUE)
}
