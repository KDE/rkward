".rk.get.meta" <- function (x) {
	c (row.names (attr (x, ".rk.meta")), as.vector (attr (x, ".rk.meta")[[1]]), recursive=TRUE)
}

".rk.set.meta" <- function (x, l, m) {
	eval (substitute (attr (x, ".rk.meta") <<- data.frame (d=m, row.names=l)))
}

".rk.editor.opened" <- function (x) {
	if (!exists (".rk.editing")) .rk.editing <<- c ()
	.rk.editing <<- c (.rk.editing, deparse (substitute (x)))
}

".rk.editor.closed" <- function (x) {
	if (exists (".rk.editing")) .rk.editing <<- .rk.editing[.rk.editing != deparse (substitute (x))]
}
#TODO: remove:
".rk.classify" <- function (x) {
	type <- 0
	if (is.data.frame (x)) type = type + 1
	if (is.matrix (x)) type = type + 2
	if (is.array (x)) type = type + 4
	if (is.list (x)) type = type + 8
	if (type != 0) type = type + 16 else type = 32
	if (is.function (x)) type = 128
	if (is.environment (x)) type = 256
	if (!is.null (attr (x, ".rk.meta"))) type = type + 2048
	d <- dim (x)
	if (length (d) < 1) d <- length (x);	# handling for objects that according to R do not have a dimension (such as vectors, functions, etc.)
	c (type, d)
}
#TODO: remove:
".rk.get.type" <- function (x) {
	if (is.data.frame (x) || is.matrix (x) || is.array (x) || is.list (x)) return (1)		# container
	if (is.function (x)) return (2)		# function
	if (is.vector (x)) return (3)		# a vector/variable
	return (4)		# something else
}

".rk.data.frame.insert.row" <- function (x, index=0) {
	if ((index == 0) || (index > dim (x)[1])) {	# insert row at end
		eval (substitute (x[dim(x)[1]+1,] <<- c (NA)))
	} else {
		for (i in dim (x)[1]:index) {
			eval (substitute (x[i+1,] <<- x[i,]))
		}
		eval (substitute (row.names (x) <<- c (1:dim(x)[1])))
		eval (substitute (x[index,] <<- c (NA)))
	}
}

# TODO: is there a nice way to get rid of a row without removing the meta-data?
".rk.data.frame.delete.row" <- function (x, index) {
	attriblist <- list ()
	for (i in 1:dim (x)[2]) {
		attriblist[[names (x)[i]]] <- attr (x[[i]], ".rk.meta")
	}
	eval (substitute (x <<- x[-index,]))
	eval (substitute (row.names (x) <<- c (1:dim(x)[1])))
	for (i in 1:dim (x)[2]) {
		eval (substitute (attr (x[[i]], ".rk.meta") <<- attriblist[[names (x)[i]]]))
	}
}

# function below is only needed to ensure a nice ordering of the columns. Simply adding a new column would be much easier than this.
".rk.data.frame.insert.column" <- function (x, label, index=0) {
	if ((index == 0) || (index > dim (x)[2])) {	# insert column at end
		eval (substitute (x[[label]] <<- c (NA)))
	} else {
		for (i in dim (x)[2]:index) {
			eval (substitute (x[i+1] <<- x[[i]]))
			eval (substitute (names (x)[i+1] <<- names (x)[i]))
		}
		eval (substitute (x[index] <<- c (NA)))
		eval (substitute (names (x)[index] <<- label))
	}
}

".rk.do.error" <- function () {
# comment in R sources says, it may not be good to query options during error handling. But what can we do, if R_ShowErrorMessages is not longer exported?
	if (getOption ("show.error.messages")) {
		.Call ("rk.do.error", c (geterrmessage ()));
	}
}

".rk.do.call" <- function (x, args=NULL) {
	.Call ("rk.do.command", c (x, args));
	if (exists (".rk.rkreply")) {
		return (.rk.rkreply)
	} else {
		return (NULL)
	}
}

# package information formats may - according to the help - be subject to change. Hence this function to cope with "missing" values
# also it concatenates everything to a single vector, so we can easily get the whole structure with a single call
".rk.get.installed.packages" <- function () {
	x <- as.data.frame (installed.packages ())
	try (titles <- as.data.frame (library ()$results)$Title)
	if (length (titles) != dim (x)[1]) titles <- rep ("", dim (x)[1])
	return (list (as.character (x$Package), as.character (titles), as.character (x$Version), as.character (x$LibPath)))
}

# Here we concatenate everything (same as above) to get search results easily
".rk.get.search.results" <- function (pattern, ...) {
	H=as.data.frame (help.search(pattern, ...)$matches)
	return(c(as.vector(H$topic),as.vector(H$title),as.vector(H$Package)))
}

# This function works like available.packages (with no arguments), but does simple caching of the result, and of course uses a cache if available. Cache is only used, if it is less than 1 hour old, and options("repos") is unchanged.
".rk.cached.available.packages" <- function () {
	x <- NULL
	if (exists (".rk.available.packages.cache")) {
		if (.rk.available.packages.cache$timestamp > (Sys.time () - 3600)) {
			if (all (.rk.available.packages.cache$repos$repos == options ("repos")$repos)) {
				x <- .rk.available.packages.cache$cache
			}
		}
	}

	if (is.null(x)) {
		x <- available.packages()
		.rk.available.packages.cache <<- list (cache = x, timestamp = Sys.time (), repos = options ("repos"))
	}

	return (x)
}

".rk.get.old.packages" <- function () {
	x <- old.packages (available=.rk.cached.available.packages ())
	return (list (as.character (x[,"Package"]), as.character (x[,"LibPath"]), as.character (x[,"Installed"]), as.character (x[,"ReposVer"]), rk.make.repos.string ()))
}

".rk.get.available.packages" <- function () {
	x <- .rk.cached.available.packages ()
	return (list (as.character (x[,1]), as.character (x[,2]), rk.make.repos.string ()))
}

"require" <- function (package, quietly = FALSE, character.only = FALSE, ...)
{
    if (!character.only) {
        package <- as.character(substitute(package))
    }
        if (!base::require(as.character(package), quietly = quietly, character.only = TRUE, ...)) {
            .rk.do.call("require", as.character(package))
            return(base::require(as.character(package), quietly = TRUE, character.only = TRUE, ...))
        }
    return(TRUE)
}

# overriding q, to ask via GUI instead. Arguments are not interpreted.
"q" <- function (save = "default", status = 0, runLast = TRUE) {
	.rk.do.call ("quit")
}

"quit" <- function (save = "default", status = 0, runLast = TRUE) {
	q (save, status, runLast)
}

#".rk.init.handlers" <- function () {
#	options (warning.expression = expression ())
#	.Internal (.addCondHands (c ("message", "warning", "error"), list (function (m) { .Call ("rk.do.condition", c ("m", conditionMessage (m))) }, function (w) { .Call ("rk.do.condition", c ("w", conditionMessage (w))) }, function (e) { .Call ("rk.do.condition", c ("e", conditionMessage (e))) }), globalenv (), NULL, TRUE))
#}

".rk.catch.window" <- function (title_begin, corresponding_device) {
	.rk.do.call ("catchWindow", c (as.character (title_begin), as.character (corresponding_device)))
}

# overriding x11 to get informed, when a new x11 window is opened
#"x11" <- function (display = "", width = 7, height = 7, pointsize = 12, gamma = 1, colortype = getOption("X11colortype"), maxcubesize = 256, bg #= "transparent", canvas = "white", fonts = getOption("X11fonts")) {
#	.rk.do.call ("startOpenX11", as.character (dev.cur ()));
#
#	if (display == "" && .Platform$GUI == "AQUA" && Sys.getenv("DISPLAY") == "")
#		Sys.putenv (DISPLAY = ":0")
#	.Internal(X11(display, width, height, pointsize, gamma, colortype, maxcubesize, bg, canvas, fonts, NA))
#
#	.rk.do.call ("endOpenX11", as.character (dev.cur ()));
#}


# changed to allow assignment of values not in levels without losing information.
"[<-.factor" <- function (x, i, value) {
	ok <- TRUE
	lx <- levels(x)
	cx <- oldClass(x)
	if (is.factor(value))
		value <- levels(value)[value]
	m <- match(value, lx)
	if (any(is.na(m) & !is.na(value))) {
		m <- value
		ok <- FALSE
		mode (x) <- "character"
		warning("invalid factor level. Dropping factor-class. Restore using rk.restore.factor ().")
	}
## here, let m revert to original value to allow temporary storage in different type (probably character)
## change storage back to 'normal' factor using "match (unclass (x), levels (x))"
	class(x) <- NULL
	if (missing(i))
		x[] <- m
	else x[i] <- m
	attr(x, "levels") <- lx
	if (ok) {
		class (x) <- cx
	} else {
		tx <- cx[cx != "factor"]
		if (length (tx) < 1) {
			class (x) <- mode (x)
		} else {
			class (x) <- tx
		}
	}
	x
}

"rk.restore.factor" <- function (x) {
	t <- match (x, levels (x))
	if (length (class (x)) > 1) {
		classes <- c ("factor", class (x))
	} else {
		classes <- "factor"
	}
	attribs <- attributes (x)
	eval (substitute (x <<- t))
	eval (substitute (attributes (x) <<- attribs))
	eval (substitute (class (x) <<- classes))
}

# these functions (not fully functional, yet) can be used to track assignments to R objects. The main interfaces are .rk.watch.symbol (k) and .rk.unwatch.symbol (k). This works by copying the symbol to a backup location, removing it, and replacing it by an active binding to the backup location
.rk.watched.value.change <- function (k, value) {
	print (paste ("set", as.character(k)))
	.rk.watched.symbols[[as.character(k)]] <<- value
}

.rk.watched.value.retrieve <- function (k) {
	print (paste ("ret", as.character(k)))
	.rk.watched.symbols[[as.character(k)]]
}

.rk.make.watch.f <- function (k) {
	function (value) {
		if (!missing (value)) {
			.rk.watched.value.change (k, value)
			invisible (value)
		}
		else {
			.rk.watched.value.retrieve (k)
		}
	}
}

.rk.watch.symbol <- function (k) {
	f <- .rk.make.watch.f (substitute (k))
	if (!exists (".rk.watched.symbols")) .rk.watched.symbols <<- list ()
	.rk.watched.symbols[[as.character (substitute (k))]] <<- k
	lst <- c (substitute (k))
	rm (list=as.character (lst), envir=parent.frame ())

	makeActiveBinding (substitute (k), f, parent.frame ())

	invisible (TRUE)
}

.rk.unwatch.symbol <- function (k) {
	lst <- c (substitute (k))
	rm (list=as.character (lst), envir=parent.frame ())

	eval (substitute (k <<- .rk.watched.symbols[[as.character (substitute (k))]]))

	.rk.watched.symbols[as.character(substitute (k))] <<- NULL

	invisible (TRUE)
}

".rk.get.structure" <- function (x, name) {
	fun <- FALSE
	cont <- FALSE
	type <- 0

# Do not change the order! Make sure all fields exist, even if empty
	ret = list ()

# 1: name should always be first
	ret$name <- as.character (name)

# 2: classification
	if (is.data.frame (x)) type = type + 1
	if (is.matrix (x)) type = type + 2
	if (is.array (x)) type = type + 4
	if (is.list (x)) type = type + 8
	if (type != 0) {
		type = type + 16
		cont <- TRUE
	} else type = 32
	if (is.function (x)) {
		fun <- TRUE
		type = 128
	}
	if (is.environment (x)) type = 256
	if (!is.null (attr (x, ".rk.meta"))) type = type + 2048
	ret$type <- as.integer (type)

# 3: classes
	ret$classes <- class (x)
	if (is.null (ret$classes)) ret$classes = ""

# 4: meta info
	ret$meta <- .rk.get.meta (x)
	if (is.null (ret$meta)) ret$meta <- ""

# 5: dimensionality
	ret$dims <- dim(x)
	if (is.null (ret$dims)) ret$dims <- length (x)	# handling for objects that - according to R - do not have a dimension (such as vectors, functions, etc.)
	if (is.null (ret$dims)) ret$dims <- 0	# according to help ("length"), we need to play safe
	ret$dims <- as.integer (ret$dims)

# 6: Special info valid for some objects ony. This should always be last in the returned structure, as the number of fields may vary
	if (cont) {		# a container
		nms <- names (x)
		if (!is.null (nms)) {
			i <- 0
			sub <- list ()
			for (child in x) {
				i <- i+1
				sub[[nms[i]]] <- .rk.get.structure (child, nms[i])
			}
			ret$sub <- sub
		}
	} else if (fun) {	# a function
		ret$argnames <- as.character (names (formals (x)))
		ret$argvalues <- as.character (formals (x))
	}

	ret
}

".rk.get.environment.structure" <- function (x) {
	ret <- list ()

	lst <- ls (x, all.names=TRUE)
	for (childname in lst) {
		ret[[childname]] <- .rk.get.structure (get (childname, envir=x), childname)
	}

	ret
}
