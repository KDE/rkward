## Internal functions manipulating graphics should be stored here.
## These functions are _not_ supposed to be called by the end user.

# overriding x11 to get informed, when a new x11 window is opened
"rk.screen.device" <- function (..., is.being.duplicated = FALSE, is.preview.device = FALSE) {
	.rk.do.call ("startOpenX11", as.character (dev.cur ()));

	old_dev <- dev.cur ()

	if (!exists (".rk.default.device")) {
		if (base::.Platform$OS.type == "unix") {
			device <- grDevices::x11
		} else {
			device <- grDevices::windows
		}
	} else {
		device <- .rk.default.device
		if (is.character (.rk.default.device)) {
			device <- get (.rk.default.device)
		}
	}
	x <- device (...)

	.rk.do.call ("endOpenX11", as.character (dev.cur ()));

	if (getOption ("rk.enable.graphics.history"))
	rk.record.plot$onAddDevice (old_dev, dev.cur (), is.being.duplicated, is.preview.device)

	invisible (x)
}

"x11" <- rk.screen.device

"X11" <- x11

if (base::.Platform$OS.type == "windows") {
	  "windows" = rk.screen.device
	  "win.graph" = rk.screen.device
}

# set from rkward the application:
# options(device="rk.screen.device")

".rk.preview.devices" <- list ();

".rk.startPreviewDevice" <- function (x) {
	a <- .rk.preview.devices[[x]]
	if (is.null (a)) {
		a <- dev.cur ()
		x11 (is.preview.device = TRUE)
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
		if (a %in% dev.list ()) {
			dev.off (a)
		}
		.rk.preview.devices[[x]] <<- NULL
	}
}

"plot.new" <- function () 
{
	if (dev.cur() == 1) rk.screen.device ()
	if (getOption ("rk.enable.graphics.history")) {
rk.record.plot$.my.message ("------- call begin -----------")
	.callstr <- sys.call (-sys.parents()[sys.nframe ()])
	rk.record.plot$record (nextplot.pkg = "graphics", nextplot.call = .callstr)
rk.record.plot$.my.message ("------- call end   -----------")
	}
	eval (body (.rk.plot.new.default))
}
formals (plot.new) <- formals (graphics::plot.new)
.rk.plot.new.default <- graphics::plot.new

"dev.off" <- function (which = dev.cur ())
{
	if (getOption ("rk.enable.graphics.history"))
	rk.record.plot$onDelDevice (devId = which)
	
	# see http://thread.gmane.org/gmane.comp.statistics.rkward.devel/802
	.rk.do.call ("killDevice", as.character (which))
	
	ret <- eval (body (.rk.dev.off.default))
	return (ret)
}
formals (dev.off) <- formals (grDevices::dev.off)
.rk.dev.off.default <- grDevices::dev.off

# see .rk.fix.assignmetns () in internal.R
".rk.fix.assignments.graphics" <- function ()
{
	assignInNamespace ("plot.new", plot.new, envir=as.environment ("package:graphics"))
	assignInNamespace ("dev.off", dev.off, envir=as.environment ("package:grDevices"))
	
	## set a hook defining "print.function" for lattice:
	setHook (packageEvent ("lattice", "onLoad"),
		function (...)
			lattice::lattice.options (print.function = function (x, ...)
			{
				if (dev.cur() == 1) rk.screen.device ()
				## TODO: use "trellis" instead of "lattice" to accomodate ggplot2 plots?
				if (getOption ("rk.enable.graphics.history")) {
rk.record.plot$.my.message ("------- call begin -----------")
				rk.record.plot$record (nextplot.pkg = "lattice")
rk.record.plot$.my.message ("------- call end   -----------")
				}
				plot (x, ...)
				if (getOption ("rk.enable.graphics.history")) {
rk.record.plot$.my.message ("------- call begin -----------")
				rk.record.plot$.save.tlo.in.hP ()
rk.record.plot$.my.message ("------- call end   -----------")
				}
				invisible ()
			})
	)
	
	## persp does not call plot.new (), so set a hook. Fortunately, the hook is placed after drawing the plot.
	setHook ("persp",
		function (...)
		{
			if (dev.cur() == 1) rk.screen.device ()
			if (getOption ("rk.enable.graphics.history")) {
rk.record.plot$.my.message ("------- call begin -----------")
			.callstr <- sys.call (-which.max(sys.parents()))
			rk.record.plot$record (nextplot.pkg = "graphics", nextplot.call = .callstr)
rk.record.plot$.my.message ("------- call end   -----------")
			}
		},
		action = "append"
	)
}

