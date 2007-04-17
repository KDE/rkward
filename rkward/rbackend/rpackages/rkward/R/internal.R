".rk.get.meta" <- function (x) {
	y <- attr (x, ".rk.meta");
	c (names (y), as.character (y))
}

".rk.set.meta" <- function (x, m) {
	eval (substitute (attr (x, ".rk.meta") <<- m))
}

".rk.set.invalid.field" <- function (x, r, d) {
	l <- attr (x, ".rk.invalid.fields");
	if (is.null (l)) l <- list ();
	l[[as.character(r)]] <- d;
	eval (substitute (attr (x, ".rk.invalid.fields") <<- l))
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

".rk.available.packages.cache" <- NULL
# This function works like available.packages (with no arguments), but does simple caching of the result, and of course uses a cache if available. Cache is only used, if it is less than 1 hour old, and options("repos") is unchanged.
".rk.cached.available.packages" <- function () {
	x <- NULL
	if (exists (".rk.available.packages.cache") && (!is.null (.rk.available.packages.cache))) {
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
"q" <- function (save = "default", status = 0, runLast = TRUE, ...) {
	.rk.do.call ("quit")
}

"quit" <- function (save = "default", status = 0, runLast = TRUE, ...) {
	q (save, status, runLast, ...)
}

#".rk.init.handlers" <- function () {
#	options (warning.expression = expression ())
#	.Internal (.addCondHands (c ("message", "warning", "error"), list (function (m) { .Call ("rk.do.condition", c ("m", conditionMessage (m))) }, function (w) { .Call ("rk.do.condition", c ("w", conditionMessage (w))) }, function (e) { .Call ("rk.do.condition", c ("e", conditionMessage (e))) }), globalenv (), NULL, TRUE))
#}

# overriding x11 to get informed, when a new x11 window is opened
"x11" <- function (...) {
	.rk.do.call ("startOpenX11", as.character (dev.cur ()));

	x <- grDevices::X11 (...)

	.rk.do.call ("endOpenX11", as.character (dev.cur ()));

	invisible (x)
}

"X11" <- x11

# these functions can be used to track assignments to R objects. The main interfaces are .rk.watch.symbol (k) and .rk.unwatch.symbol (k). This works by copying the symbol to a backup environment, removing it, and replacing it by an active binding to the backup location
".rk.watched.symbols" <- new.env ()

# override makeActiveBinding: If active bindings are created in globalenv (), watch them properly
"makeActiveBinding" <- function (sym, fun, env, ...) {
	if (identical (env, globalenv ())) {
		base::makeActiveBinding (sym, fun, .rk.watched.symbols, ...)
		f <- .rk.make.watch.f (sym)
		base::makeActiveBinding (sym, f, globalenv (), ...)
	} else {
		base::makeActiveBinding (sym, fun, env, ...)
	}
}

".rk.make.watch.f" <- function (k) {
	# we need to make sure, the functions we use are *not* looked up as symbols in .GlobalEnv.
	# else, for instance, if the user names a symbol "missing", and we try to resolve it in the
	# wrapper function below, evaluation would recurse to look up "missing" in the .GlobalEnv
	# due to the call to "if (!missing(value))".
	get <- base::get
	missing <- base::missing
	assign <- base::assign
	.rk.do.call <- rkward::.rk.do.call
	invisible <- base::invisible

	function (value) {
		if (!missing (value)) {
			assign (k, value, envir=.rk.watched.symbols)
			.rk.do.call ("ws", k);
			invisible (value)
		} else {
			get (k, envir=.rk.watched.symbols)
		}
	}
}

".rk.watch.symbol" <- function (k) {
	f <- .rk.make.watch.f (k)
	assign (k, get (k, envir=globalenv ()), envir=.rk.watched.symbols)
	rm (list=k, envir=globalenv ())

	base::makeActiveBinding (k, f, globalenv ())

	invisible (TRUE)
}

# not needed by rkward
".rk.unwatch.symbol" <- function (k) {
	rm (list=k, envir=globalenv ())

	k <<- .rk.watched.symbols$k

	rm (k, envir=.rk.watched.symbols);

	invisible (TRUE)
}

".rk.watch.globalenv" <- function () {
	newlist <- ls (globalenv (), all.names=TRUE)
	oldlist <- ls (.rk.watched.symbols, all.names=TRUE)
	for (old in oldlist) {		# unwatch no longer present items
		if (!(old %in% newlist)) {
			rm (list=old, envir=.rk.watched.symbols);
		}
	}

	for (new in newlist) {		# watch new items
		if (!(new %in% oldlist)) {
			.rk.watch.symbol (new)
		}
	}
}

".rk.get.vector.data" <- function (x) {
	ret <- list ();
	ret$data <- as.vector (unclass (x));
	ret$levels <- levels (x)
	if (is.null (ret$levels)) ret$levels <- ""
	i <- attr (x, ".rk.invalid.fields")
	ret$invalids <- as.character (c (names (i), i));
	if (length (ret$invalids) == 0) ret$invalids <- ""
	ret
}

# Change storage type of x to mode newmode.
# Most attributes will be kept, but the data is erased!
".rk.set.vector.mode" <- function (x, fun, envir=parent.frame ()) {
	old_attr <- attributes (x)
	old_attr$class <- NULL
	old_attr[[".rk.invalid.fields"]] <- list ()	# will be reset, anyway!

	y <- fun (rep (NA, length.out=length (x)))

	# merge old attributes with new ones
	newattrs <- attributes (y)
	for (nattr in names (newattrs)) {
		old_attr[[nattr]] <- newattrs[[nattr]]
	}

	attributes (y) <- old_attr
	eval (substitute (x <- y), envir=envir)
}

".rk.get.structure.old" <- function (x, name, envlevel=0, namespacename=NULL, misplaced=FALSE, envir) {
	fun <- FALSE
	cont <- FALSE
	type <- 0

# Do not change the order! Make sure all fields exist, even if empty
	if (missing (x)) {
#		.Call ("rk.test.type", name, envir) # Testing code. TODO: clean up
		x <- get (name, envir=envir)
	}

# 1: name should always be first
	name <- as.character (name)

# 2: classification
	if (is.data.frame (x)) type <- type + 1
	if (is.matrix (x)) type <- type + 2
	if (is.array (x)) type <- type + 4
	if (is.list (x)) type <- type + 8
	if (type != 0) {
		type <- type + 16
		cont <- TRUE
	} else {
		if (is.function (x)) {
			fun <- TRUE
			type <- 128
		} else if (is.environment (x)) {
			type <- 256
			cont <- TRUE
		} else {
			type <- 32
			if (is.factor (x)) type <- type + 32768			# 2 << 14
			else if (is.numeric (x)) type <- type + 16384		# 1 << 14
			else if (is.character (x)) type <- type + 49152		# 3 << 14
			else if (is.logical (x)) type <- type + 65536		# 4 << 14
		}
	}
	if (!is.null (attr (x, ".rk.meta"))) type = type + 4096
	if (misplaced) type <- type + 8192
	type <- as.integer (type)

# 3: classes
	classes <- class (x)
	if (is.null (classes)) classes = ""

# 4: meta info
	meta <- .rk.get.meta (x)
	if (is.null (meta)) meta <- ""

# 5: dimensionality
	dims <- dim(x)
	if (is.null (dims)) dims <- length (x)	# handling for objects that - according to R - do not have a dimension (such as vectors, functions, etc.)
	if (is.null (dims)) dims <- 0	# according to help ("length"), we need to play safe
	dims <- as.integer (dims)

# 6: Special info valid for some objects ony. This should always be last in the returned structure, as the number of fields may vary
	if (cont) {		# a container
		if (is.environment (x)) {
			sub <- .rk.get.environment.children (x, envlevel+1, namespacename)
		} else {
			sub <- list ()
			nms <- names (x)
			if (!is.null (nms)) {
				i <- 0
				for (child in x) {
					i <- i+1
					sub[[nms[i]]] <- .rk.get.structure (child, nms[i], envlevel)
				}
			}
		}
		return (invisible (list (name, type, classes, meta, dims, sub)))
	} else if (fun) {	# a function
		argnames <- as.character (names (formals (x)))
		argvalues <- as.character (lapply (formals (x), function (v) {
						if (is.character (v)) return (encodeString (v, quote="\""))
						else return (v)
					} ))
		return (invisible (list (name, type, classes, meta, dims, argnames, argvalues)))
	}
	return (invisible (list (name, type, classes, meta, dims)))
}

".rk.get.structure.new" <- function (x, name, envlevel=0, namespacename=NULL) {
	.Call ("rk.get.structure.test", x, name, namespacename)
}

".rk.get.structure" <- .rk.get.structure.old

".rk.get.formals" <- function (x) 
{
    f <- formals (x)
    r <- as.character(lapply(f, function(v) {
        if (is.character(v)) return(encodeString(v, quote = "\"")) else return(v)
    }))
    names (r) <- names (f)
    r
}

".rk.get.environment.children" <- function (x, envlevel=0, namespacename=NULL) {
	ret <- list ()

	if (envlevel < 2) {		# prevent infinite recursion
		lst <- ls (x, all.names=TRUE)
		if (is.null (namespacename)) {
			for (childname in lst) {
				ret[[childname]] <- .rk.get.structure (name=childname, envlevel=envlevel, envir=x)
			}
		} else {
			# before R 2.4.0, operator "::" would only work on true namespaces, not on package names (operator "::" work, if there is a namespace, and that namespace has the symbol in it)
			# TODO remove once we depend on R >= 2.4.0
			if (compareVersion (paste (R.version$major, R.version$minor, sep="."), "2.4.0") < 0) {
				ns <- tryCatch (asNamespace (namespacename), error = function(e) NULL)
				for (childname in lst) {
					misplaced <- FALSE
					if (is.null (ns) || (!exists (childname, envir=ns, inherits=FALSE))) misplaced <- TRUE
					ret[[childname]] <- .rk.get.structure (name=childname, envlevel=envlevel, misplaced=misplaced, envir=x)
				}
			} else {
			# for R 2.4.0 or greater: operator "::" works if package has no namespace at all, or has a namespace with the symbol in it
				ns <- tryCatch (asNamespace (namespacename), error = function(e) NULL)
				for (childname in lst) {
					misplaced <- FALSE
					if ((!is.null (ns)) && (!exists (childname, envir=ns, inherits=FALSE))) misplaced <- TRUE
					ret[[childname]] <- .rk.get.structure (name=childname, envlevel=envlevel, misplaced=misplaced, envir=x)
				}
			}
		}
	}

	ret
}

".rk.output.html.file" <- NULL

".rk.rkreply" <- NULL

".rk.set.reply" <- function (x) .rk.rkreply <<- x

".rk.preview.devices" <- list ();

".rk.startPreviewDevice" <- function (x) {
	a <- .rk.preview.devices[[x]]
	if (is.null (a)) {
		a <- dev.cur ()
		x11 ()
		if (a != dev.cur ()) {
			.rk.preview.devices[[x]] <<- dev.cur ()
		}
	} else {
		dev.set (a)
	}
}

".rk.killPreviewDevice" <- function (x) {
	a <- .rk.preview.devices[[x]]
	if (!is.null (a)) {
		.rk.preview.devices[[x]] <<- NULL
		if (a %in% dev.list ()) {
			dev.off (a)
		}
	}
}

"Sys.setlocale" <- function (category = "LC_ALL", locale = "", ...) {
	if (category == "LC_ALL" || category == "LC_CTYPE" || category == "LANG") {
		.rk.do.call ("preLocaleChange", NULL);
		if (!is.null (.rk.rkreply)) {
			if (.rk.rkreply == FALSE) stop ("Changing the locale was cancelled by user");
		}

		ret <- base::Sys.setlocale (category, locale, ...)

		.Call ("rk.update.locale")
		ret
	} else {
		base::Sys.setlocale (category, locale, ...)
	}
}

"setwd" <- function () {
	eval (body (base::setwd))
	.rk.do.call ("wdChange", NULL);
}
formals (setwd) <- formals (base::setwd)

# hidden, as this is not portable to different output formats
".rk.cat.output" <- function (x) {
	cat (x, file = rk.get.output.html.file(), append = TRUE)
}
