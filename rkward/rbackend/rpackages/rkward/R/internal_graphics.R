## Internal functions manipulating graphics should be stored here.
## These functions are _not_ supposed to be called by the end user.

# overriding x11 to get informed, when a new x11 window is opened
"rk.screen.device" <- function (..., is.being.duplicated = FALSE, is.preview.device = FALSE) {
	.rk.do.call ("startOpenX11", as.character (dev.cur ()));

	old_dev <- dev.cur ()

	args <- list (...)
	if (!exists (".rk.default.device")) {
		if (base::.Platform$OS.type == "unix") {
			device <- grDevices::x11
		} else {
			device <- grDevices::windows
			if (is.null (args[["width"]])) args[["width"]] <- options ("rk.screendevice.width")[[1]]
			if (!is.numeric (args[["width"]])) args[["width"]] <- 7
			if (is.null (args[["height"]])) args[["height"]] <- options ("rk.screendevice.height")[[1]]
			if (!is.numeric (args[["height"]])) args[["height"]] <- 7
		}
	} else {
		device <- .rk.default.device
		if (is.character (.rk.default.device)) {
			device <- get (.rk.default.device)
		}
	}
	x <- do.call (device, args)

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

".rk.preview.devices" <- list ()

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

.rk.variables$.rk.printer.devices <- list ()

# see .rk.fix.assignmetns () in internal.R
".rk.fix.assignments.graphics" <- function ()
{
	rk.replace.function ("plot.new", as.environment ("package:graphics"),
		function () {
			rk.record.plot$.plot.new.hook ()
			eval (body (.rk.backups$plot.new))
		})

	rk.replace.function ("dev.off", as.environment ("package:grDevices"),
		function (which = dev.cur ()) {
			if (getOption ("rk.enable.graphics.history"))
				rk.record.plot$onDelDevice (devId = which)
			
			# see http://thread.gmane.org/gmane.comp.statistics.rkward.devel/802
			.rk.do.call ("killDevice", as.character (which))
			
			ret <- eval (body (.rk.backups$dev.off))

			printfile <- .rk.variables$.rk.printer.devices[[as.character (which)]]
			if (!is.null (printfile)) {
				.rk.do.plain.call ("printPreview", printfile, FALSE)
				.rk.variables$.rk.printer.devices[[as.character (which)]] <- NULL
			}

			return (ret)
		})

	rk.replace.function ("dev.set", as.environment ("package:grDevices"),
		function () {
			ret <- eval (body (.rk.backups$dev.set))
			
			if (getOption ("rk.enable.graphics.history") && rk.record.plot$.is.device.managed (which))
				rk.record.plot$.set.trellis.last.object (which)
			
			ret
		})

	## set a hook defining "print.function" for lattice:
	setHook (packageEvent ("lattice", "onLoad"),
		function (...)
			lattice::lattice.options (print.function = function (x, ...)
			{
				if (dev.cur() == 1) rk.screen.device ()
				## TODO: use "trellis" instead of "lattice" to accomodate ggplot2 plots?
				if (getOption ("rk.enable.graphics.history")) {
					rk.record.plot$record (nextplot.pkg = "lattice")
				}
				plot (x, ...)
				if (getOption ("rk.enable.graphics.history")) {
					rk.record.plot$.save.tlo.in.hP ()
				}
				invisible ()
			})
	)

	setHook (packageEvent ("grid", "attach"),
		function (...)
			rk.replace.function ("grid.newpage", as.environment ("package:grid"),
				function () {
					## TODO: add specific support for ggplots?
					rk.record.plot$.plot.new.hook ()
					ret <- eval (body (.rk.backups$grid.newpage))
				})
	)

	## persp does not call plot.new (), so set a hook. Fortunately, the hook is placed after drawing the plot.
	setHook ("persp",
		function (...)
		{
			rk.record.plot$.plot.new.hook ()
		},
		action = "append"
	)
}

