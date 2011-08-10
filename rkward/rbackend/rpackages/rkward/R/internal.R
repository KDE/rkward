".rk.get.meta" <- function (x) {
	y <- attr (x, ".rk.meta");
	c (names (y), as.character (y))
}

".rk.set.meta" <- function (x, m) {
	eval (substitute (attr (x, ".rk.meta") <<- m))
}

".rk.set.invalid.field" <- function (x, row, value) {
	l <- attr (x, ".rk.invalid.fields");
	if (is.null (l)) l <- list ();
	l[[as.character(row)]] <- value;
	if (length (l) == 0) l <- NULL
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
		attriblist[[names (x)[i]]] <- attributes (x[[i]])
	}
	newx <- x[-index, , drop=FALSE]
	row.names (newx) <- c (1:dim(newx)[1])
	for (i in 1:dim (newx)[2]) {
		attributes (newx[[i]]) <- attriblist[[names (newx)[i]]]
	}
	eval (substitute (x <<- newx))
}

# function below is only needed to ensure a nice ordering of the columns. Simply adding a new column would be much easier than this.
".rk.data.frame.insert.column" <- function (x, label, index=0) {
	column <- rep (as.numeric (NA), times=dim (x)[1])
	if ((index == 0) || (index > dim (x)[2])) {	# insert column at end
		eval (substitute (x[[label]] <<- column))
	} else {
		for (i in dim (x)[2]:index) {
			eval (substitute (x[i+1] <<- x[[i]]))
			eval (substitute (names (x)[i+1] <<- names (x)[i]))
		}
		eval (substitute (x[index] <<- column))
		eval (substitute (names (x)[index] <<- label))
	}
}

".rk.do.error" <- function () {
# comment in R sources says, it may not be good to query options during error handling. But what can we do, if R_ShowErrorMessages is not longer exported?
	if (getOption ("show.error.messages")) {
		.Call ("rk.do.error", c (geterrmessage ()));
	}
}

".rk.set.reply" <- function (x) .rk.variables$.rk.rkreply <- x

".rk.do.call" <- function (x, args=NULL) {
	.rk.set.reply (NULL)
	.Call ("rk.do.command", c (x, args));
	return (.rk.variables$.rk.rkreply)
}

".rk.do.plain.call" <- function (x, args=NULL, synchronous=TRUE) {
	.Call ("rk.do.generic.request", c (x, args), isTRUE (synchronous))
}

".rk.find.package.pluginmaps" <- function (package, all.maps=FALSE) {
	if(isTRUE(all.maps)){
		# look for all pluginmaps in the rkward folder
		pluginmaps <- dir(system.file("rkward", package=package), pattern="*.pluginmap", full.names=TRUE)
	} else {
		# check if a main .pluginmap file is provided
		pluginmaps <- system.file(file.path("rkward", paste(package, ".pluginmap", sep="")), package=package)
	}
	return(pluginmaps)
}

# package information formats may - according to the help - be subject to change. Hence this function to cope with "missing" values
# also it concatenates everything to a single vector, so we can easily get the whole structure with a single call
".rk.get.installed.packages" <- function () {
	x <- as.data.frame(installed.packages(fields="Title"))
	# does a package enhance RKWard, i.e. provide plugins?
	enhance.rk <- ifelse(is.na(x$Enhances), FALSE, grepl("rkward", x$Enhances))
	pluginmaps <- ifelse(enhance.rk, .rk.find.package.pluginmaps(x$Package), "")
	return(list(Package=as.character(x$Package), Title=as.character(x$Title), 
		Version=as.character(x$Version), LibPath=as.character(x$LibPath),
		EnhanceRK=as.logical(enhance.rk), Plugins=as.character(pluginmaps)))
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
		invisible(base::require(as.character(package), quietly = TRUE, character.only = TRUE, ...))
	} else {
		invisible(TRUE)
	}
}

# overriding q, to ask via GUI instead. Arguments are not interpreted.
"q" <- function (save = "default", status = 0, runLast = TRUE, ...) {
	res <- .rk.do.plain.call ("quit")
	if (length (res) && (res == "FALSE")) stop ("Quitting was cancelled")
}

"quit" <- function (save = "default", status = 0, runLast = TRUE, ...) {
	q (save, status, runLast, ...)
}

#".rk.init.handlers" <- function () {
#	options (warning.expression = expression ())
#	.Internal (.addCondHands (c ("message", "warning", "error"), list (function (m) { .Call ("rk.do.condition", c ("m", conditionMessage (m))) }, function (w) { .Call ("rk.do.condition", c ("w", conditionMessage (w))) }, function (e) { .Call ("rk.do.condition", c ("e", conditionMessage (e))) }), globalenv (), NULL, TRUE))
#}

# these functions can be used to track assignments to R objects. The main interfaces are .rk.watch.symbol (k) and .rk.unwatch.symbol (k). This works by copying the symbol to a backup environment, removing it, and replacing it by an active binding to the backup location
".rk.watched.symbols" <- new.env ()

# override makeActiveBinding: If active bindings are created in globalenv (), watch them properly
.rk.makeActiveBinding.default <- base::makeActiveBinding
"makeActiveBinding" <- function (sym, fun, env, ...) {
	if (identical (env, globalenv ())) {
		.rk.makeActiveBinding.default (sym, fun, .rk.watched.symbols, ...)
		f <- .rk.make.watch.f (sym)
		.rk.makeActiveBinding.default (sym, f, globalenv (), ...)
	} else {
		.rk.makeActiveBinding.default (sym, fun, env, ...)
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
	.Call ("rk.copy.no.eval", k, globalenv(), .rk.watched.symbols);
	#assign (k, get (k, envir=globalenv ()), envir=.rk.watched.symbols)
	rm (list=k, envir=globalenv ())

	.rk.makeActiveBinding.default (k, f, globalenv ())

	invisible (TRUE)
}

# not needed by rkward but provided for completeness
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

".rk.get.structure" <- function (x, name, envlevel=0, namespacename=NULL) {
	.Call ("rk.get.structure", x, as.character (name), as.integer (envlevel), namespacename)
}

".rk.try.get.namespace" <- function (name) {
	tryCatch (asNamespace (name), error = function(e) NULL)
}

".rk.get.structure.global" <- function (name, envlevel=0, namespacename=NULL) {
	.Call ("rk.get.structure.global", as.character (name), as.integer (envlevel), namespacename)
}

".rk.get.slots" <- function (x) {
	slotnames <- methods::slotNames (class (x))
	ret <- lapply (slotnames, function (slotname) slot (x, slotname))
	names (ret) <- slotnames
	ret
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
			# for R 2.4.0 or greater: operator "::" works if package has no namespace at all, or has a namespace with the symbol in it
			ns <- tryCatch (asNamespace (namespacename), error = function(e) NULL)
			for (childname in lst) {
				misplaced <- FALSE
				if ((!is.null (ns)) && (!exists (childname, envir=ns, inherits=FALSE))) misplaced <- TRUE
				ret[[childname]] <- .rk.get.structure (name=childname, envlevel=envlevel, misplaced=misplaced, envir=x)
			}
		}
	}

	ret
}

"Sys.setlocale" <- function (category = "LC_ALL", locale = "", ...) {
	if (category == "LC_ALL" || category == "LC_CTYPE" || category == "LANG") {
		allow <- .rk.do.plain.call ("preLocaleChange", NULL)
		if (length (allow) && (allow == "FALSE")) stop ("Changing the locale was cancelled by user")

		ret <- base::Sys.setlocale (category, locale, ...)

		.Call ("rk.update.locale")
		ret
	} else {
		base::Sys.setlocale (category, locale, ...)
	}
}

"setwd" <- function () {
	ret <- eval (body (base::setwd))
	.rk.do.plain.call ("wdChange", base::getwd (), synchronous=FALSE)
	invisible (ret)
}
formals (setwd) <- formals (base::setwd)

# hidden, as this is not portable to different output formats
".rk.cat.output" <- function (x) {
	cat (x, file = rk.get.output.html.file(), append = TRUE)
}

".rk.rerun.plugin.link" <- function (plugin, settings, label) {
	.rk.cat.output (paste ("<a href=\"rkward://runplugin/", plugin, "/", URLencode (settings), "\">", label, "</a>", sep=""))
}

".rk.make.hr" <- function () {
	.rk.cat.output ("<hr>\n");
}

# Start recording commands that are submitted from rkward to R.
# filename: filename to write to (file will be truncated!).
# include.all: By default, some types of command are filtered (internal synchronisation commands, and run again links). Should these be included?
# To stop recording, supply NULL or "" as filename
# Currently used for the purpose of automated testing, only. Perhaps in the future
# this or a similar mechanism could also be added as a user feature.
"rk.record.commands" <- function (filename, include.all = FALSE) {
	if (is.null (filename)) filename = ""

	res <- .rk.do.plain.call ("recordCommands", c(as.character (filename), if (include.all) "include.all" else "normal"))

	if (!length (res)) invisible (TRUE)
	else {
		warning (res)
		invisible (FALSE)
	}
}

# General purpose storage environment (which will hopefully never get locked by R)
".rk.variables" <- new.env ()
.rk.variables$.rk.active.device <- 1
.rk.variables$.rk.output.html.file <- NULL
.rk.variables$.rk.rkreply <- NULL

".rk.backups" <- new.env ()
# Tries to replace a function inside its environemnt/namespace.
# Function formals are copied from the original.
# A backup of the original is stored as rkward::.rk.backups$FUNCTIONNAME
"rk.replace.function" <- function (functionname, environment, replacement, copy.formals=TRUE) {
	original <- get (functionname, envir=environment, inherits=FALSE)

	# create a backup
	assign (functionname, original, envir=.rk.backups)

	if (copy.formals) formals (replacement) <- formals (original)
	environment (replacement) <- environment (original)
	try (
		if (bindingIsLocked (functionname, environment)) {
			unlockBinding (functionname, environment)
			on.exit (lockBinding (functionname, environment))
		}
	)
	try (
		if (isNamespace (environment)) {
			assignInNamespace (functionname, replacement, ns=environment)
		} else {
			assignInNamespace (functionname, replacement, envir=environment)
		}
	)
	try (
		assign (functionname, replacement, envir=environment)
	)

	invisible (NULL)
}

# where masking is not enough, we need to assign in the original environment / namespace. This can only be done after package loading,
# so we have a separate function for that.
".rk.fix.assignments" <- function () {
	## History manipulation function (overloads for functions by the same name in package utils)
	rk.replace.function ("loadhistory",  as.environment ("package:utils"),
		function (file = ".Rhistory") {
			invisible (.rk.do.plain.call ("commandHistory", c ("set", readLines (file))))
		}, copy.formals = FALSE)

	rk.replace.function ("savehistory",  as.environment ("package:utils"),
		function (file = ".Rhistory") {
			invisible (writeLines (.rk.do.plain.call ("commandHistory", "get"), file))
		}, copy.formals = FALSE)

	rk.replace.function ("timestamp",  as.environment ("package:utils"),
		function (stamp = date(), prefix = "##------ ", suffix = " ------##", quiet = FALSE) {
			stamp <- paste(prefix, stamp, suffix, sep = "")
			.rk.do.plain.call ("commandHistory", c ("append", stamp))
			if (!quiet) cat(stamp, sep = "\n")
			invisible(stamp)
		}, copy.formals = FALSE)

	## Interactive menus
	rk.replace.function ("select.list", as.environment ("package:utils"), 
		function () {
			# the "list" parameter was renamed to "choices" in R 2.11.0
			if (!exists ("list", inherits=FALSE)) list <- choices
			# the "graphics" parameter was introduced in R 2.11.0, so we cannot rely on its existance
			if (!exists ("graphics", inherits=FALSE)) graphics <- TRUE
			if (graphics) {
				return (rk.select.list (list, preselect, multiple, title))
			}

			# for text list, use the default implementation
			eval (body (.rk.backups$select.list))
		})

	rk.replace.function ("menu", as.environment ("package:utils"),
		function () {
			if (graphics) {
				res <- rk.select.list (choices, multiple=FALSE, title=title)
				return (match(res, choices, nomatch = 0L))
			}

			# for text menus, use the default implementation
			eval (body (.rk.backups$menu))
		})

	# call separate assignments functions:
	if (exists (".rk.fix.assignments.graphics")) eval (body (.rk.fix.assignments.graphics)) # internal_graphics.R
}

