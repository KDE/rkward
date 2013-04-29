## Public functions manipulating "graphics" should be stored here.
## These functions are accessible to the user.

#' Plot graphics to HTML output file
#'
#' \code{rk.graph.on()} creates an R device that will plot to the output html page in RKWard (\url{rkward://page/rkward_output}).
#' The default settings for \code{device.type}, \code{width}, \code{height}, and \code{quality} can be modified from Settings -> Configure RKWard -> Output.
#'
#' @param device.type Type of device to create / graphics format to use. Currently, supported devices are "PNG", "SVG", or "JPG".
#'                    The default is to use the format configured in Settings -> Configure RKWard -> Output.
#' @param width Width of graphics in pixels. The default is to use the width configured in Settings -> Configure RKWard -> Output.
#' @param height Height of graphics in pixels. The default is to use the heigth configured in Settings -> Configure RKWard -> Output.
#' @param quality For device.type "JPG", quality of the JPG file from 0 to 100.
#'                The default is to use the quality configured in Settings -> Configure RKWard -> Output.
#' @param ... Further options will be passed to the graphics device used internally.
#'
#' \bold{Warning}: It is advised to use \code{rk.graph.off} and \bold{not} \code{dev.off} to close the device opened by
#' \code{rk.graph.on}. \code{dev.print(device = rk.graph.on)} is a \bold{wrong} usage for this "device," and will result in errors.
#' 
#' @author Thomas Friedrichsmeier \email{rkward-devel@@lists.sourceforge.net}
#' 
#' @seealso \link{rk.results} \link{rk.print} \link{rk.get.output.html.file} \link{dev.off} \link{svg} \link{png} \link{jpg}
#'
#' @examples
#' require (rkward)
#' 
#' ## Plot directly to the output (html) file, by-passing screen device:
#' rk.graph.on ("JPG", 480, 480, 75)
#' plot (rnorm (100))
#' rk.graph.off ()
#' 
#' ## Copy the displayed plot to the output:
#' plot (rnorm (100))
#' dev.copy (device = rk.graph.on)
#' rk.graph.off ()
#' 
#' ## WRONG USAGE: not run:
#' #plot (rnorm (100))
#' #dev.print (device = rk.graph.on)
#'
#' @keywords devices
#'
#' @export
#' @aliases rk.graph.on rk.graph.off
#' @rdname rk.graph.on
"rk.graph.on" <- function (device.type=getOption ("rk.graphics.type"), width=getOption ("rk.graphics.width"), height=getOption ("rk.graphics.height"), quality, ...) 
{
	if (!is.numeric (width)) width <- 480
	if (!is.numeric (height)) height <- 480
	if (is.null (device.type)) device.type <- "PNG"	# default behavior is PNG for now

	assign (".rk.active.device", dev.cur (), .rk.variables)

	ret <- NULL
	if (device.type == "PNG") {
		filename <- rk.get.tempfile.name(prefix = "graph", extension = ".png")
		ret <- png(filename = file.path(filename), width = width, height = height, ...)
		.rk.cat.output(paste("<img src=\"", filename, "\" width=\"", width, 
			"\" height=\"", height, "\"><br>", sep = ""))
	} else if (device.type == "JPG") {
		if (missing (quality)) {
			quality = getOption ("rk.graphics.jpg.quality")		# COMPAT: getOption (x, *default*) not yet available in R 2.9
			if (is.null (quality)) quality = 75
		}
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

#' \code{rk.graph.off()} closes the device that was opened by \code{rk.graph.on}. 
#'
#' @rdname rk.graph.on
#' @export
"rk.graph.off" <- function(){
	.rk.cat.output ("\n")	# so the output will be auto-refreshed
	ret <- dev.off()
	
	# dev.off () sets dev.next () as active, which may not have been active before rk.graph.on was called;
	# so reset the correct device as active:
	i <- get (".rk.active.device", .rk.variables)
	if ((!is.null (i)) && (i %in% dev.list ())) ret <- dev.set (i)
	ret
}

#' Plot graphics to RKWard native device
#'
#' \code{RK()} creates an R on-screen device that will be rendered in the RKWard frontend. 
#' The default settings for \code{width}, and \code{height} can be modified from Settings -> Configure RKWard -> Onscreen Graphics.
#'
#' @param width Width of the device in inches. The default is to use the heigth configured in Settings -> Configure RKWard -> Onscreen Graphics.
#' @param height Height of the device in inchesgraphics in pixels. The default is to use the heigth configured in Settings -> Configure RKWard -> Onscreen Graphics.
#' @param pointsize Default pointsize
#' @param family Default font family. This can be a character vector of length 1 or 2. The second value is used for
#'               plotting symbols. Effectively the default is c("Helvetica", "Symbol"). A wide variety of sepcification is supported,
#'               including the device independent fonts names "sans", "serif", and "mono"
#' @param bg Background color.
#' @param title Window title.
#' @param antialias Antialiasing. Can be turned off for somewhat faster drawing.
#'
#' @keywords devices
#'
#' @export
#' @aliases RK
#' @rdname RKdevice
"RK" <- function (width=getOption("rk.screendevice.width"), height=getOption("rk.screendevice.height"), pointsize=12, family=NULL, bg="white", title="", antialias=TRUE) {
	if (is.null (width)) width <- 7
	if (is.null (height)) height <- 7
	ret <- .Call ("rk.graphics.device", as.integer (width), as.integer (height), as.integer (pointsize), family, bg, title, isTRUE (antialias), PACKAGE="(embedding)")
	rk.record.plot$onAddDevice (dev.cur ())
	invisible (ret)	# Current always NULL
}

#' Embed non-RKWard device windows
#'
#' \code{rk.embed.device} evaluates the given expression, and if this has created a window on the screen, tries to embed it as an RKWard window.
#'
#' @param expr  Expression to evaluate. 
#'
#' @details Theoretically, \code{expr} can be any valid R expression. However typically this should be calls to X11(), Windows(), or, perhaps dev.copy().
#'       Importantly, the expression should create exactly one new window for \code{rk.embed.device()} to work. Keep in mind, that this is not
#'       always the case for \code{plot(...)} and similar commands, which will re-use an existing plot window, if available.
#'
#' @note \code{rk.embed.device()} will not work on all platforms (most importantly, not in most MacOSX binaries). Further, note that a captured
#'       \code{X11()} or \code{Windows} device may look similar to an \code{RK()} device, but is actually a very different thing.
#'
#' @seealso \link{RK}
#'
#' @export
"rk.embed.device" <- function (expr) {
	oldd <- dev.cur ()
	.rk.do.call ("startOpenX11", as.character (oldd));
	on.exit (.rk.do.call ("endOpenX11", as.character (dev.cur())));

	x <- eval.parent (expr)

	if (oldd != dev.cur ()) on.exit (rk.record.plot$onAddDevice (), add=TRUE)
	else warning ("No device appears to have been created (dev.cur() has not changed)");
	invisible (x)
}

# Internal function to create wrapper around an R device function (used for X11(), windows(), and quartz()).
".rk.make.device.wrapper" <- function (devicename) {
	ret <- eval (substitute (
		function (width=getOption("rk.screendevice.width"), height=getOption("rk.screendevice.height"), pointsize=12) {
			rk.mode <- getOption ("rk.override.platform.devices")
			if (identical (rk.mode, "replace") || !exists (devicename, envir=asNamespace ("grDevices"), inherits=FALSE)) {
				if (!identical (rk.mode, "replace")) {
					warning (paste (devicename, "()-device is not available. Falling back to RK()"))
				}
				args <- list ()
				if (exists ("width", inherits=FALSE) && !missing (width)) args$width <- width
				if (exists ("height", inherits=FALSE) && !missing (height)) args$height <- height
				if (exists ("pointsize", inherits=FALSE) && !missing (pointsize)) args$pointsize <- pointsize
				if (exists ("bg", inherits=FALSE) && !missing (bg)) args$bg <- bg
				if (exists ("title", inherits=FALSE) && !missing (title)) args$title <- title
				if (exists ("antialias", inherits=FALSE) && !missing (antialias)) args$antialias <- !identical (antialias, "none")
				do.call (rkward::RK, args)
			} else if (identical (rk.mode, "embed")) {
				if (missing (width)) width <- getOption ("rk.screendevice.width")
				if (!is.numeric (width)) width <- 7
				if (missing (height)) height <- getOption ("rk.screendevice.height")
				if (!is.numeric (height)) height <- 7
				rk.embed.device (eval (body (grDevices::devicename)))
			} else {
				eval (body (grDevices::devicename))
			}
		}
	))
	if (exists (devicename, envir=asNamespace ("grDevices"), inherits=FALSE)) {
		devfun <- get (devicename, asNamespace ("grDevices"))
		formals (ret) <- formals (devfun)
		environment (ret) <- environment (devfun)
	}
	ret
}

## NOTE: Adding an Rd-page for these makes "?X11" fail (R 3.0.0).
# Overrides for platform specific R plotting devices
#
# These functions override the platform specific on-screen plotting devices by the same names.
# The exact behavior depends on configuration settings, and can be one of: The original R device,
# the original R device embedded using \code{rk.embed.device()}, or the call can be re-directed
# to the \code{RK()} device. In the last case not all function arguments may be honored.
#
# @note If you want to use the \link{RK} device, you should call that, explicitly. These
#       overrides are provided to make it easy to use scripts that refer to the platform specific
#       plotting devices provided by R.
#
# @seealso \link{RK} \link{rk.embed.device} \link[grDevices]{X11} \link[grDevices]{Windows}
#
# @rdname DeviceOverrides
#' @export
"X11" <- .rk.make.device.wrapper ("X11")
#' @export
"x11" <- X11
#' @export
"windows" <- .rk.make.device.wrapper ("windows")
#' @export
"win.graph" <- .rk.make.device.wrapper ("win.graph")  # NOTE: Has different formals() than windows()
#' @export
"quartz" <- .rk.make.device.wrapper ("quartz")

#' Device for printing using the KDE print dialog
#' 
#' Creates a device operating on a temporary file (internally a
#' \code{\link{postscript}}() device). When the device is closed, it is
#' printed, automatically, using the KDE print dialog (if installed).
#' 
#' Typically this device is used with \code{\link{dev.print}}, as shown in the
#' example, below.
#' 
#' @param ... arguments are passed to \code{\link{postscript}}
#' @return Returns the name of the underlying temporary file, invisibly.
#' @author Thomas Friedrichsmeier \email{rkward-devel@@lists.sourceforge.net}
#' @seealso \code{\link{postscript}}, \code{\link{dev.print}},
#'   \code{\link{rk.graph.on}}
#' @keywords utilities device
#' @rdname rk.printer.device
#' @export
#' @examples
#' 
#' ## Not run:
#' plot (rnorm (10))
#' dev.print (rk.printer.device)
#' 
# Produces a temporary postscript file and opens a print dialog for it
# Parameters are passed to postscript(), but typically this is simply used as
#   dev.print(rk.print.preview)
"rk.printer.device" <- function(...) {
	tf <- paste (tempfile (), "ps", sep=".")	# NOTE: Up to R 2.12.x, tempfile() did not have "fileext" parameter
	postscript (file = tf, ...)
	.rk.variables$.rk.printer.devices[[as.character (dev.cur ())]] <- tf
}

#' @export
"rk.duplicate.device" <- function (devId = dev.cur ())
{
	rk.record.plot$duplicating.from.device <- devId
	on.exit (rk.record.plot$duplicating.from.device <- 1)	# NULL device
	dev.set (devId)
	dev.copy (device = dev.new)
}

# A global history of various graphics calls;
#' @export
"rk.record.plot" <- function ()
{
	env <- environment()
	
	# .sP.index is used maintain an index of the history using Sys.time. This will help
	# when "insert"ing a plot into history is implemented. We then have to shift around
	# only .sP.index and not the whole "savedPlots" list
	.sP.index <- list ()
	sP.length <- length (.sP.index)
	.set.sP.length <- function () sP.length <<- length (.sP.index)
	
	# template for every element of savedPlots; tlo.ls is ("lattice.status") used only for lattice plots
	.sP.template <- list (plot = NULL, tlo.ls = NULL, pkg = "", time = NULL, call = NULL)
	# this is the main list which stores all the history; the list is tagged by Sys.time
	savedPlots <- list () # length (savedPlots) should always be == length (.sP.index) == sP.length
	# used for temporarily storing the plots before they are pushed into savedPlots:
	.unsavedPlot <- list (plot = NULL, tlo.ls = NULL, pkg = NA_character_, is.os = NA, tryerr = NA)
	
	# template for every element of histPositions; tlo.ls ("lattice.status") is used only for lattice plots
	.hP.template <- list (is.this.plot.new = FALSE, is.this.dev.new = TRUE, 
		pos.cur = NA_integer_, pos.prev = NA_integer_, pos.dupfrom = NA_integer_, 
		pkg = "", call = NA_character_, plot = NA, tlo.ls = NA)
	# this list stores the details for currently displayed plots on the devices; tagged by device number
	histPositions <- list ("1" = .hP.template)
	.hP.names <- names (histPositions)
	.set.hP.names <- function () .hP.names <<- names (histPositions)
	
	.ss.used <- FALSE # split.screen variable
	.pop.notify <- TRUE # used when hist limit is reached
	.cll <- 50 # no. of characters used in the "goto plot" drop down list
	.set.call.lab.len <- function (x) .cll <<- x
	
	## Generic functions:
	.get.sys.time <- function () format (Sys.time (), "%Y%m%d%H%M%OS3")
	.is.device.managed <- function (devId) as.character (devId) %in% .hP.names[-1]
	.set.trellis.last.object <- function (devId = dev.cur ())
	{
		# called only from dev.set (); this appropriately sets the "lattice.status"
		# list so that trellis.last.object () can retrieve the correct variables
		if (!.is.device.managed (devId)) return (invisible ())
		devId <- as.character (devId)
		if (histPositions[[devId]]$pkg != "lattice") return (invisible ())
		
		if (histPositions[[devId]]$is.this.plot.new)
			tlo.ls <- histPositions[[devId]]$tlo.ls
		else
			tlo.ls <- savedPlots [[.sP.index [[histPositions[[devId]]$pos.cur]]]]$tlo.ls
		
		assign ("lattice.status", tlo.ls, envir = lattice:::.LatticeEnv)
	}
	.is.par.or.screen.inuse <- function ()
	{
		# takes care of par (mfrow / mfcol) and split.screen () issues "almost!"
		ret <- FALSE
		if (sum (par ("mfg") * c(-1,-1,1,1)) != 0)
			ret <- TRUE
		else if (graphics:::.SSexists ("sp.screens")) {
			if (!.ss.used)
				.ss.used <<- TRUE
			else
				ret <- TRUE
		} else 
			.ss.used <<- FALSE
		ret
	}
	
	## Device specific functions:
	onAddDevice <- function (devId = dev.cur ())
	{
		if (!isTRUE (getOption ("rk.enable.graphics.history"))) return (invisible ())
		
		devId.from <- as.character (env$duplicating.from.device)
		devId <- as.character (devId)
		
		histPositions [[devId]] <<- .hP.template
		if ((env$duplicating.from.device > 1) && !histPositions [[devId.from]]$is.this.dev.new) {
			# devId.from > 1
			## TODO: see if so many "[[" calls can be reduced?
			histPositions [[devId]]$is.this.plot.new <<- TRUE
			histPositions [[devId]]$is.this.dev.new <<- FALSE
			histPositions [[devId]]$pkg <<- histPositions [[devId.from]]$pkg
			histPositions [[devId]]$call <<- histPositions [[devId.from]]$call
			if (!histPositions [[devId.from]]$is.this.plot.new)
				histPositions [[devId]]$pos.dupfrom <<- histPositions [[devId.from]]$pos.cur
			histPositions [[devId]]$plot <<- histPositions [[devId.from]]$plot
			histPositions [[devId]]$tlo.ls <<- histPositions [[devId.from]]$tlo.ls
		}
		.set.hP.names ()
		.rk.update.hist.actions ()
		invisible ()
	}
	initialize.histPositions <- function ()
	{
		# this is called from rk.toggle.plot.history ();
		# when plot history is re-enabled, this initializes device specific lists so that the displayed
		# plots can be recorded at the next appropriate action
		on.exit (.rk.update.hist.actions (enable.plot.hist = TRUE))
		
		# all open screen devices
		.osd <- which (names (dev.list ()) %in% deviceIsInteractive ()) + 1
		.opd <- .rk.list.preview.device.numbers()
		# to be managed devices:
		if (length (.opd) > 0) .osd <-.osd [!(.osd %in% .opd)]
		if (length (.osd) == 0) return (invisible ())
		
		d.cur <- dev.cur ()
		histPositions <<- list ("1" = .hP.template)
		for (d in as.character (.osd)) {
			.rk.backups$dev.set (as.numeric (d))
			if (is.null (recordPlot ()[[1]])) # empty device
				histPositions [[d]] <<- .hP.template
			else
				histPositions [[d]] <<- modifyList(.hP.template, 
					list (is.this.plot.new = TRUE, is.this.dev.new = FALSE, pkg = "unknown"))
		}
		.rk.backups$dev.set (d.cur)
		.set.hP.names ()
	}
	onDelDevice <- function (devId = dev.cur())
	{
		devId <- as.character (devId)
		if (!(devId %in% .hP.names[-1])) return (invisible ())
		
		## TODO: ask for confirmation before appending a plot
		record (devId, action = "dev.off")
		histPositions [[devId]] <<- NULL
		.set.hP.names ()
		
		invisible ()
	}
	flushout.histPositions <- function ()
	{
		# this is called from rk.toggle.plot.history ()
		# when plot history is disabled, this records any unsaved plots on the devices and
		# cleans out the device specific lists
		
		# save any unsaved plots and "close" the device w/o actually closing the window:
		for (d in .hP.names)
			record (devId = d, action = "dev.off")
		.rk.update.hist.actions (enable.plot.hist = FALSE)
		histPositions <<- list ("1" = .hP.template)
	}
	.save.tlo.in.hP <- function (devId = dev.cur ())
	{
		if (!.is.device.managed (devId)) return (invisible ())
		# tlo = trellis.last.object
		# when there are multiple devices showing the same lattice plot in the history, we need to
		# store the "lattice.status" into each device specific list, so that, if/when removing
		# one of the displayed plots, the other can still be re-added back in the history.
		devId <- as.character (devId)
		histPositions [[devId]]$plot <<- trellis.last.object ()
		histPositions [[devId]]$tlo.ls <<- get ("lattice.status", envir = lattice:::.LatticeEnv)
		invisible ()
	}
	.prep.new.device <- function (devId, pkg, .cstr)
	{
		histPositions [[devId]]$is.this.dev.new <<- FALSE
		histPositions [[devId]]$is.this.plot.new <<- TRUE
		histPositions [[devId]]$pkg <<- pkg
		histPositions [[devId]]$call <<- .cstr
		invisible ()
	}
	
	## Recording functions
	record <- function(devId = dev.cur (), isManaged = NULL, action = "", callUHA = TRUE, nextplot.pkg = "", nextplot.call = NA_character_)
	{
		# callUHA is not really utilized, but there to provide a flixibility to not call 
		# .rk.update.hist.action () when not needed
		devId <- as.character (devId)
		
		if (is.null (isManaged)) isManaged <- .is.device.managed (devId)
		if (!isManaged) return (invisible ())
		
		if (histPositions[[devId]]$is.this.dev.new) {
			# a new device: ie after either an "x11 ()" call or a "dev.copy (device = x11)" call
			if (action == "")  
				return (invisible (.prep.new.device (devId, nextplot.pkg, nextplot.call))) # call from plot.new () / persp () / print.trellis ()
			else if (action == "force.append")
				return (invisible (rk.show.message ("Nothing to record!", "Record Warning", FALSE))) # call from rk.force.append.plot
			else
				return (invisible ()) # if needed, handle individual actions separately
		}
		
		newplot.in.Q <- nextplot.pkg != ""
		if (action == "force.append") {
			histPositions[[devId]]$is.this.plot.new <<- TRUE
			histPositions[[devId]]$pkg <<- "unknown"
			histPositions[[devId]]$call <<- NA_character_
		} else if (nextplot.pkg == "graphics") {
			# unless force.append is used,
			# check for par (mfrow / mfcol / mfg) and split.screen scenarios:
			if (.is.par.or.screen.inuse () && action != "dev.off") return (invisible ())
		}
		st <- .get.sys.time ()
		n <- switch (histPositions[[devId]]$pkg,
			graphics = .record.graphics (devId, action, newplot.in.Q, st),
			unknown = .record.graphics (devId, action, newplot.in.Q, st, "unknown"),
			lattice = .record.lattice (devId, action, newplot.in.Q, st),
			NA_integer_)
		
		if (newplot.in.Q) {
			.tmp.hP <- modifyList (.hP.template, 
				list (is.this.plot.new = TRUE, is.this.dev.new = FALSE, pkg = nextplot.pkg, call = nextplot.call))
			.tmp.hP$pos.prev <- ifelse (is.null (.unsavedPlot$plot) && .unsavedPlot$is.os, 
				histPositions [[devId]]$pos.prev, n)
			histPositions [[devId]] <<- .tmp.hP
		} else {
			histPositions [[devId]]$is.this.plot.new <<- FALSE
			if (!is.na (n)) histPositions [[devId]]$pos.cur <<- n
			if (action == "force.append") histPositions [[devId]]$plot <<- NA
		}
		
		if (callUHA) .rk.update.hist.actions ()
		invisible ()
	}
	.record.graphics <- function (devId, action, newplot.in.Q, st, pkg = "graphics")
	{
		.record.main (devId, pkg)
		if (is.null (.unsavedPlot$plot)) return (invisible (NA_integer_))
		
		if (histPositions [[devId]]$is.this.plot.new) {
			save.mode <- ifelse (newplot.in.Q, "append", action)
			if (save.mode %in% c("arrows", "dev.off", "force.append")) save.mode <- "append"
		} else {
			save.mode <- ifelse (newplot.in.Q, "overwrite", action)
			if (save.mode %in% c("arrows", "dev.off")) save.mode <- "overwrite"
		}
		
		n <- save.plot.to.history (devId, save.mode, 
			ifelse (action == "force.append", "unknown", pkg), 
			st, histPositions[[devId]]$call)
		invisible (n)
	}
	.record.lattice <- function (devId, action, newplot.in.Q, st)
	{
		if (!histPositions [[devId]]$is.this.plot.new) return (invisible (histPositions [[devId]]$pos.cur))
		
		.record.main (devId, "lattice")
		if (is.null (.unsavedPlot$plot)) return (invisible (NA_integer_))
		
		save.mode <- ifelse (newplot.in.Q, "append", action)
		if (save.mode %in% c("arrows", "dev.off")) save.mode <- "append"
		
		n <- save.plot.to.history (devId, save.mode, "lattice", st, histPositions[[devId]]$call)
		invisible (n)
	}
	.record.main <- function (devId, pkg)
	{
		devId.cur <- dev.cur ()
		unsplot <- NULL
		unsplot.ls <- NULL
		if (pkg %in% c("graphics", "unknown")) {
			.rk.backups$dev.set (as.numeric (devId))
			try (unsplot <- recordPlot(), silent=TRUE)
			.rk.backups$dev.set (devId.cur)
		} else if  (pkg == "lattice") {
			unsplot <- histPositions [[devId]]$plot
			unsplot.ls <- histPositions [[devId]]$tlo.ls
		} else {
			.unsavedPlot <<- list (plot = NULL, tlo.ls = NULL, pkg = NA_character_, is.os = NA, tryerr = NA)
			return (invisible (rk.show.message ("Unknown graphics function. Use append to store.", "Recording error", FALSE)))
		}
		
		if (class (unsplot) == "try-error") {
			.unsavedPlot <<- list (plot = NULL, tlo.ls = NULL, pkg = pkg, is.os = NA, tryerr = TRUE)
			return (invisible (rk.show.message ("Unknown recording error", "Recording error", FALSE)))
		}
		
		.unsavedPlot <<- list (plot = unsplot, tlo.ls = unsplot.ls, pkg = pkg, 
			is.os = object.size (unsplot) > getOption ("rk.graphics.hist.max.plotsize") * 1024, tryerr = FALSE)
		
		invisible ()
	}
	
	## Saving (the recorded plot) functions:
	save.plot.to.history <- function (devId, save.mode, pkg, st, .cstr = NA_character_)
	{
		switch (save.mode,
			append = .save.plot.to.history.append (devId, pkg, st, .cstr),
			overwrite = .save.plot.to.history.overwrite (devId, pkg, st, .cstr),
			NA_integer_)
	}
	.save.plot.to.history.append <- function (devId, pkg, st, .cstr)
	{
		if (!.save.oversized.plot ()) return (invisible (NA_integer_))
		
		n <- .grow.history (st)
		if (is.na (n)) return (invisible (n))
		
		savedPlots [[st]] <<- list (plot = .unsavedPlot$plot, tlo.ls = .unsavedPlot$tlo.ls, pkg = pkg, time = st, call = NULL)
		savedPlots [[st]]$call <<- try (.get.oldplot.call (n, .cll, .cstr))
		invisible (n)
	}
	.save.plot.to.history.overwrite <- function (devId, pkg, st, .cstr)
	{
		# this is not setup to handle an (yet unwritten) 'overwrite' action from tool/menu bar
		n <- histPositions [[devId]]$pos.cur
		.st. <- .sP.index [[n]]
		if (!.check.identical (.st., pkg) && !is.null (.unsavedPlot$plot)) {
			if (!.save.oversized.plot ()) return (invisible (n))
			savedPlots [[.st.]]$plot <<- .unsavedPlot$plot
			savedPlots [[.st.]]$tlo.ls <<- .unsavedPlot$tlo.ls
			savedPlots [[.st.]]$call <<- try (.get.oldplot.call (n, .cll, .cstr))
			.check.other.dev.at.same.pos (devId, n)
		}
		invisible (n)
	}
	.save.oversized.plot <- function ()
	{
		if (is.na (.unsavedPlot$is.os))
			ret <- FALSE
		else if (!.unsavedPlot$is.os)
			ret <- TRUE
		else 
			ret <- rk.show.question ("Large plot!\nDo you still want to store it in the history?", 
				"WARNING!", button.cancel = "")
		ret
	}
	.check.identical <- function (.st., pkg=NA_character_) {
		# this may need to be split into separate .check.identical."pkg" functions
		identical (savedPlots[[.st.]]$plot, .unsavedPlot$plot)
	}
	.check.other.dev.at.same.pos <- function (devId, .n.)
	{
		# length (.n.) >= 1 when .verify.hist.limits () calls remove ()
		odnames <- .hP.names [!(.hP.names %in% c("1", devId))]
		if (length (odnames) == 0) return (invisible ())
		
		odpos <- sapply (histPositions [odnames], "[[", "pos.cur") # names kept
		odpos <- odpos [which (odpos %in% .n.)]
		if (length (odpos) == 0) return (invisible ())
		
		for (d in names (odpos)) {
			histPositions[[d]]$is.this.plot.new <<- TRUE
			histPositions[[d]]$pos.prev <<- histPositions[[d]]$pos.cur ## may not be approprite for "remove"
			histPositions[[d]]$pos.cur <<- NA_integer_
		}
		invisible ()
	}
	.grow.history <- function (st)
	{
		len.sP <- sP.length
		ml <- getOption ('rk.graphics.hist.max.length')
		
		if (len.sP < ml) {
			n <- len.sP + 1
		} else if (len.sP == ml) {
			if (.pop.notify)
				.pop.notify <<- rk.show.question ("History limit reached, removing the first plot. Limits can be changed at Settings > RKWard > Output.\n\nDo you want to be notified in future?", 
					"WARNING!", button.cancel = "")
			remove (devId = NULL, pos = 1) # sP.length changes at this point
			n <- len.sP
		} else {
			# this can happen, if max history length gets set below sP.length, without removing the excess plots,
			# still, this should be avoided.
			rk.show.message ("Current history length > max length: plot not added to history!", "WARNING!")
			return (invisible (NA_integer_))
		}
		.sP.index [[n]] <<- st
		.set.sP.length ()
		n
	}
	
	## Removal function:
	remove <- function (devId = dev.cur (), pos = NA_integer_) # pos can be of length > 1
	{
		# devId == NULL when called from verify.hist.length ()
		
		if (sP.length == 1) {
			clearHistory ()
			rk.show.message ("Plot history cleared!", "Remove plot", FALSE)
		}
		if (sP.length <= 1) return (invisible ())
		
		if (!is.null (devId)) devId <- as.character (devId)
		
		if (!is.null (devId)) {
			if (histPositions[[devId]]$is.this.dev.new) # on an empty device
				return (invisible (rk.show.message ("Nothing to remove!", "Remove plot", FALSE)))
			else if (is.na (pos) || histPositions[[devId]]$is.this.plot.new) {
				# unsaved plot on the device, so just replay the "previous" plot
				.p. <- histPositions[[devId]]$pos.prev
				if (is.na (.p.)) .p. <- sP.length
				replay (.p., devId)
				return (invisible ())
			}
		}
		
		.check.other.dev.at.same.pos (devId, pos) # works for devId = NULL as well
		
		.sP.pos <- unlist (.sP.index [pos])
		savedPlots [.sP.pos] <<- NULL
		.sP.index [pos] <<- NULL
		.set.sP.length ()
		
		if (!is.null (devId)) replay (min (pos, sP.length), devId) # in this case, length (pos) == 1
		
		.l. <- length (pos)
		hP.gt.pos <- sapply (histPositions, "[[", "pos.cur")
		hP.gt.pos <- hP.gt.pos [which (hP.gt.pos > pos[.l.])] # removes NAs; pos[.l.] == max (pos)
		if (length (hP.gt.pos) > 0) {
			for (.d. in names (hP.gt.pos)) {
				histPositions[[.d.]]$pos.cur <<- min (histPositions [[.d.]]$pos.cur - .l., sP.length)
				histPositions[[.d.]]$pos.prev <<- min (histPositions [[.d.]]$pos.prev - .l., sP.length)
			}
		}
		
		.rk.update.hist.actions ()
		invisible ()
	}
	clearHistory <- function ()
	{
		.sP.index <<- list (); .set.sP.length ()
		savedPlots <<- list ()
		.unsavedPlot <<- list (plot = NULL, tlo.ls = NULL, pkg = NA_character_, is.os = NA, tryerr = NA)
		.ss.used <<- FALSE
		for (d in .hP.names[-1]) {
			if (!histPositions [[d]]$is.this.dev.new)
				histPositions [[d]]$is.this.plot.new <<- TRUE
			histPositions [[d]]$pos.cur <<- NA_integer_
			histPositions [[d]]$pos.prev <<- NA_integer_
			histPositions [[d]]$pos.dupfrom <<- NA_integer_
			# do not reset "pkg" and "call" elements
		}
		.rk.update.hist.actions ()
		invisible ()
	}
	
	## Replay function:
	replay <- function(n, devId = dev.cur ())
	{
		on.exit (.rk.update.hist.actions ())
		if (missing (n))
			return (invisible (rk.show.message ("Position missing", "Replay error", FALSE)))
		if (is.na (n) || n <= 0 || n > sP.length)
			return (invisible (rk.show.message(paste ("replay: 'n' not in valid range: ", n), "Replay error", FALSE)))
		
		devId <- as.character (devId)
		cur.devId <- dev.cur ()
		.rk.backups$dev.set (as.numeric(devId))
		
		st <- .sP.index [[n]]
		pkg <- savedPlots [[st]]$pkg
		
		if (pkg %in% c("graphics", "unknown")) {
			# NOTE: replayPlot() does *not* call plot.new()
			replayPlot (savedPlots [[st]]$plot)
		} else if (pkg == "lattice") {
			# (re-)plot the lattice object but, if the current window is NOT active, then do not save
			# it to lattice:::.LatticeEnv$lattice.status ("trellis.last.object" needs it). But we need
			# to set lattice.status to whichever was the last lattice plot so that trellis.last.object () can
			# access it
			if (cur.devId != as.numeric (devId))
				tlo.ls <- get ("lattice.status", envir = lattice:::.LatticeEnv)
			rk.without.plot.history ({
				plot (savedPlots [[st]]$plot, save.object = (cur.devId == as.numeric (devId)))
			})
			if (cur.devId != as.numeric (devId))
				assign ("lattice.status", tlo.ls, envir = lattice:::.LatticeEnv)
		}
		histPositions [[devId]] <<- modifyList (.hP.template, 
			list (is.this.plot.new = FALSE, is.this.dev.new = FALSE, pos.prev = n, pos.cur = n, pkg = pkg, 
				call = savedPlots [[st]]$call, plot = savedPlots [[st]]$plot, tlo.ls = savedPlots [[st]]$tlo.ls))
		.rk.backups$dev.set (cur.devId)
		invisible()
	}
	
	## Action wrappers:
	showFirst <- function(devId = dev.cur())
	{
		if (!.is.device.managed (devId)) return (invisible ())
		record (devId, isManaged = TRUE, action = "arrows")
		replay(n = 1, devId)
	}
	showPrevious <- function(devId)
	{
		if (!.is.device.managed (devId)) return (invisible ())
		record (devId, isManaged = TRUE, action = "arrows")
		.n. <- histPositions [[as.character (devId)]]$pos.cur - 1L
		if (is.na (.n.)) .n. <- sP.length # this happens when sP.length > 0 and the user calls x11 ()
		replay(n = .n., devId = devId)
	}
	showNext <- function(devId)
	{
		if (!.is.device.managed (devId)) return (invisible ())
		record (devId, isManaged = TRUE, action = "arrows")
		replay(n = histPositions [[as.character (devId)]]$pos.cur + 1L, devId = devId)
	}
	showLast <- function(devId = dev.cur())
	{
		if (!.is.device.managed (devId)) return (invisible ())
		record (devId, isManaged = TRUE, action = "arrows")
		replay(n = sP.length, devId)
	}
	showPlot <- function(devId = dev.cur(), index)
	{
		if (!.is.device.managed (devId)) return (invisible ())
		
		.n. <- histPositions [[as.character (devId)]]$pos.cur
		if (index == ifelse (is.na (.n.), sP.length + 1, .n.)) {
			# same position! No action needed
			return (invisible ())
		}
		
		record (devId, isManaged = TRUE, action = "arrows")
		index <- max (as.integer (index), 1L)
		replay(n = min (sP.length, index), devId)
	}
	forceAppend <- function (devId = dev.cur ())
	{
		if (!.is.device.managed (devId)) return (invisible (rk.show.message ("Device not managed", "Append this plot", FALSE)))
		record (devId, isManaged = TRUE, action = "force.append")
	}
	removePlot <- function (devId = dev.cur ())
	{
		if (!.is.device.managed (devId)) return (invisible (rk.show.message ("Device not managed", "Remove plot", FALSE)))
		remove (devId, histPositions[[as.character (devId)]]$pos.cur)
	}
	showPlotInfo <- function (devId = dev.cur ())
	{
		rk.show.message (.get.plot.info.str (devId), caption = "Plot properties", FALSE)
	}
	
	## Utility / print functions:
	getDevSummary <- function (devId = NULL)
	{
		message ("History length   : ", sP.length)
		message ("History size (KB): ", round (object.size (savedPlots) / 1024, 2))
		if (is.null (devId)) {
			.tmp.df <- data.frame (
				pNew = sapply (histPositions, "[[", "is.this.plot.new"),
				dNew = sapply (histPositions, "[[", "is.this.dev.new"),
				posC = sapply (histPositions, "[[", "pos.cur"),
				posP = sapply (histPositions, "[[", "pos.prev"),
				posD = sapply (histPositions, "[[", "pos.dupfrom"),
				pkg  = sapply (histPositions, "[[", "pkg"),
				pCls  = sapply (lapply (histPositions, "[[", "plot"), FUN = function (x) class (x)))
			rownames (.tmp.df) <- names (histPositions)
		} else {
			devId <- as.character (devId)
			.a.hP <- histPositions[[devId]]
			.tmp.df <- data.frame (
				pNew = .a.hP$is.this.plot.new,
				dNew = .a.hP$is.this.dev.new,
				posC = .a.hP$pos.cur,
				posP = .a.hP$pos.prev,
				posD = .a.hP$pos.dupfrom,
				pkg  = .a.hP$pkg,
				pCls  = class (.a.hP$plot))
			rownames (.tmp.df) <- devId
		}
		.tmp.df
	}
	getSavedPlotsSummary <- function ()
	{
		.tmp.df <- data.frame (
			call = sapply (savedPlots[unlist (.sP.index, use.names = FALSE)], "[[", "call"),
			size.KB  = sapply (lapply (savedPlots[unlist (.sP.index, use.names = FALSE)], "[[", "plot"), function (x) object.size(x)/1024),
			pkg  = sapply (savedPlots[unlist (.sP.index, use.names = FALSE)], "[[", "pkg"),
			timestamp  = sapply (savedPlots[unlist (.sP.index, use.names = FALSE)], "[[", "time"))
		rownames (.tmp.df) <- NULL
		.tmp.df
	}
	
	## Utility / call labels functions:
	.get.sP.calls <- function ()
	{
		labels <- NULL
		if (sP.length > 0)
			labels <- paste (format (1:sP.length), sapply (savedPlots [unlist (.sP.index, use.names = FALSE)], "[[", "call"), sep = ": ")
		names (labels) <- NULL
		labels
	}
	.get.plot.info.str <- function (devId = dev.cur (), l=0)
	{
		devId <- as.character (devId)
		if (!(devId %in% .hP.names)) return (paste ("Device", devId, "is not managed."))
		
		n <- histPositions [[devId]]$pos.cur
		if (is.na (n)) {
			info.str <- paste ("Device: ", devId, ", Position: <new>, Size: ?\nType: ", histPositions [[devId]]$pkg, sep = "")
		} else if (n == 0) {
			info.str <- paste ("Device: ", devId, ", Position: 0", sep = "")
		} else {
			info.str <- paste ("Device: ", devId, 
				", Position: ", n, 
				", Size (KB): ", round (object.size (savedPlots [[.sP.index [[n]]]]$plot)/1024, 2),
				"\n", .get.oldplot.call (n, l, histPositions [[devId]]$call), sep = "")
		}
		info.str
	}
	.get.oldplot.call <- function (n, l=0, cs = NA_character_)
	{
		# this can be easily extended to more types
		switch (savedPlots [[.sP.index [[n]]]]$pkg,
			graphics = .get.oldplot.call.std (l, cs),
			unknown = .get.oldplot.call.unk (n, l),
			lattice = .get.oldplot.call.lattice (n, l),
			"Unknown")
	}
	.get.oldplot.call.unk <- function (n,l=0)
	{
		# rp <- recordPlot () is a nested pairlist object (of class "recordedplot"):
		# rp[[1]] is the "meta data", rp[[2]] is always raw,
		# We then figure out the relevant stuff from rp[[1]]. Use "str (rp)" for details.
		# Currently, only main, sub, xlab, and ylab meta data can be extracted, unambiguously.
		# The high level calls are not part of the meta data, only the low level .Internal
		#   calls are stored: Eg: .Primitive (plot.xy), .Primitive (rect), .Primitive (persp), etc...
		
		# .f. identifies which element(s) in rp[[1]] contains title (=main,sub,xlab,ylab) information:
		# differs from call to call. Eg: for plot () calls this is 7, for hist () this is 3, ...
		.tmp.plot. <- savedPlots [[.sP.index [[n]]]]$plot[[1]]
		.f. <- function ()
			which (lapply (.tmp.plot., FUN = function (x) x[[1]]) == ".Primitive(\"title\")")
		# Sometimes there is no title information at all - happens when the high level calling function
		#   does not specifically provide any of main/sub/xlab/ylab arguemnts: Eg: persp (...)
		# Sometimes there are more than one .Primitive ("title") calls, eg, when title (...) is called
		#   explicitely after a plotting call
		
		.x. <- list (main = "", sub = "", xlab = "", ylab = "")
		
		# When present, rp [[1]] [.n.] [[2]] contains (in multiple lists) main, sub, xlab, ylab, etc.
		# From there we pick up the last (which.max) non-null entry for each of main, sub, xlab, and ylab
		.n. <- .f. ()
		if (length (.n.) > 0) {
			.T. <- lapply (lapply (.tmp.plot. [.n.], FUN = function (.a.) .a.[[2]]), 
				FUN = function (.aa.) {names (.aa.) <- c("main", "sub", "xlab", "ylab"); .aa.})
			
			for (i in c("main", "sub", "xlab", "ylab"))
				.x.[[i]] <- .T. [[ which.max (sapply (.T., FUN = function (.a.) !is.null (.a.[[i]]))) ]] [[i]]
		}
		
		#.lab.str <- paste ("Main: '", .x.$main, "'; X label: '", .x.$xlab, "'; Y label: '", .x.$ylab, "'", sep = "")
		.lab.str <- paste ("X: ", .x.$xlab, "; Y: ", .x.$ylab, "; ", .x.$main, sep = "")
		if (all (unlist (.x.) == "")) .lab.str <- paste ("<Unknown>", .lab.str)
		if (l <= 0 || nchar (.lab.str) <= l) return (.lab.str)
		
		paste (substr (.lab.str, 1, l), "...", sep = "")
	}
	.get.oldplot.call.std <- function (l=0, cs)
	{
		.lab.str <- paste (ifelse (is.call (cs), deparse (cs), cs), collapse = ifelse (l<=0, "\n", ", "))
		if (l <= 0 || nchar (.lab.str) <= l) return (.lab.str)
		
		paste (substr (.lab.str, 1, l), "...", sep = "")
	}
	.get.oldplot.call.lattice <- function (n,l=0)
	{
		.lab.str <- paste (deparse (savedPlots [[.sP.index [[n]]]]$plot$call), collapse = ifelse (l<=0, "\n", ", "))
		if (l <= 0 || nchar (.lab.str) <= l) return (.lab.str)
		
		paste (substr (.lab.str, 1, l), "...", sep = "")
	}
	.plot.new.hook <- function (.callstr) {
		if (dev.cur() == 1) dev.new ()
		if (getOption ("rk.enable.graphics.history")) {
			.callstr <- sys.call (-sys.parents()[sys.nframe ()])
			record (nextplot.pkg = "graphics", nextplot.call = .callstr)
		}
	}
	
	## Utility / R - C++ connection functions:
	.rk.update.hist.actions <- function (devIds = .hP.names, enable.plot.hist = TRUE)
	{
		# this function is called whenever the history length changes
		# or the position changes in any device.
		
		devIds <- devIds [devIds != "1"] # ignore NULL device
		ndevs <- length (devIds)
		if (ndevs > 0) {
			positions <- character (2 * ndevs)
			positions [2 * (1:ndevs) - 1] <- devIds
			ihP <- sapply (histPositions[devIds], "[[", "pos.cur"); ihP [is.na (ihP)] <- sP.length + 1
			positions [2 * (1:ndevs)] <- ihP
			.rk.do.call ("updateDeviceHistory", c (ifelse (enable.plot.hist, sP.length, 0), .get.sP.calls (), positions));
		}
		invisible ()
	}
	.verify.hist.limits <- function (len.max)
	{
		# this is called from settings/rksettingsmoduleoutput.cpp ~199
		# Length restriction:
		if (len.max < sP.length) {
			ans <- rk.show.question (paste ("Current plot history has more plots than the specified limit.\nIf you continue then _",
				sP.length - len.max, "_ of the foremost plots will be removed.\nInstead, if you ignore then the new limit will be effective only after restarting RKWard.", sep =""), 
				"WARNING!",
				button.yes = "Continue", button.no = "Ignore for this session", button.cancel = "")
			if (ans) {
				options ("rk.graphics.hist.max.length" = len.max)
				remove (devId = NULL, pos = 1:(sP.length - len.max))
			}
		} else {
			# this takes care of the initialization when RKWard starts..
			options ("rk.graphics.hist.max.length" = len.max)
		}
		
		# Size restriction:
		#s <- getOption ('rk.graphics.hist.max.plotsize')
		# Existing plots are not checked for their sizes, only the new ones are.
	}

	env$duplicating.from.device <- 1 # NULL device
	env
}
rk.record.plot <- rk.record.plot ()

# Users should use only these wrappers:
# 1 is always the null device
#' @export
"rk.toggle.plot.history" <- function (x = TRUE)
{
	if (x) {
		rk.record.plot$initialize.histPositions ()
	} else {
		rk.record.plot$flushout.histPositions ()
	}
	options ("rk.enable.graphics.history" = x)
	invisible ()
}
#' @export
"rk.first.plot" <- function (devId = dev.cur ())
{
	if (!getOption ("rk.enable.graphics.history")) return (invisible ())
	rk.record.plot$showFirst (devId)
}
#' @export
"rk.previous.plot" <- function (devId = dev.cur ())
{
	if (!getOption ("rk.enable.graphics.history")) return (invisible ())
	rk.record.plot$showPrevious (devId)
}
#' @export
"rk.next.plot" <- function (devId = dev.cur ())
{
	if (!getOption ("rk.enable.graphics.history")) return (invisible ())
	rk.record.plot$showNext (devId)
}
#' @export
"rk.last.plot" <- function (devId = dev.cur ())
{
	if (!getOption ("rk.enable.graphics.history")) return (invisible ())
	rk.record.plot$showLast (devId)
}
#' @export
"rk.goto.plot" <- function (devId = dev.cur (), index=1)
{
	if (!getOption ("rk.enable.graphics.history")) return (invisible ())
	rk.record.plot$showPlot (devId, index)
}
#' @export
"rk.force.append.plot" <- function (devId = dev.cur ())
{
	if (!getOption ("rk.enable.graphics.history")) return (invisible ())
	rk.record.plot$forceAppend (devId)
}
#' @export
"rk.removethis.plot" <- function (devId = dev.cur ())
{
	if (!getOption ("rk.enable.graphics.history")) return (invisible ())
	rk.record.plot$removePlot (devId)
}
#' @export
"rk.clear.plot.history" <- function ()
{
	if (!getOption ("rk.enable.graphics.history")) return (invisible ())
	rk.record.plot$clearHistory ()
}
#' @export
"rk.show.plot.info" <- function (devId = dev.cur ())
{
	if (!getOption ("rk.enable.graphics.history")) return (invisible ())
	rk.record.plot$showPlotInfo (devId)
}
#' @export
"rk.verify.plot.hist.limits" <- function (lmax)
{
	if (!getOption ("rk.enable.graphics.history")) return (invisible ())
	rk.record.plot$.verify.hist.limits (as.integer (lmax))
}
#' @export
"rk.plot.history.summary" <- function (which = NULL, type = c ("devices", "history"))
{
	ret <- NULL
	if (getOption ("rk.enable.graphics.history")) 
		ret <- switch (
			devices = rk.record.plot$getDevSummary (which),
			history = rk.record.plot$getSavedPlotsSummary (),
			NULL)
	ret
}
#' Run a (plotting) action, without recording anything in the plot history.
#' Internally, the plot history option is turned off for the duration of the action.
#' 
#' @export
"rk.without.plot.history" <- function (expr)
{
	if (getOption ("rk.enable.graphics.history")) {
		on.exit (options ("rk.enable.graphics.history" = TRUE))
	}
	options ("rk.enable.graphics.history" = FALSE)
	eval.parent(expr)
}
