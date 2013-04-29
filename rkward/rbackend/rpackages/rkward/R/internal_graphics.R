## Internal functions manipulating graphics should be stored here.
## These functions are _not_ supposed to be called by the end user.

#' DEPRECATED: \code{rk.screen.device} is obsolete. It simply calls \code{dev.new()} in this version of RKWard.
#'
#' Depending on your use case, you should use \code{dev.new()}, \code{RK()} or \code{rk.embed.device()}, instead.
#'
#' @seealso \link{dev.new}, \link{RK}, \link{rk.embed.device}
#'
#' @export
"rk.screen.device" <- function (...) {
	warning ("rk.screen.device() is obsolete.\nUse one of dev.new(), RK(), or rk.embed.device(), instead.")
	dev.new (...)
}

# Fetch the current size of the given RK() device from the frontend, and redraw
"RK.resize" <- function (devnum) {
	.Call ("rk.graphics.device.resize", as.integer (devnum)-1, PACKAGE="(embedding)")
}

#' @include internal.R
assign(".rk.preview.devices", list (), envir=.rk.variables)

#' @export
".rk.startPreviewDevice" <- function (x) {
	a <- .rk.variables$.rk.preview.devices[[x]]
	if (is.null (a)) {
		devnum <- dev.cur ()
		rk.without.plot.history (dev.new ())
		if (devnum != dev.cur ()) {
			.rk.variables$.rk.preview.devices[[x]] <- list (devnum=dev.cur(), par=par (no.readonly=TRUE))
		} else {
			return (0L)	# no device could be opened
		}
	} else {
		dev.set (a$devnum)
		par (a$par)
	}
	as.integer (dev.cur ())
}

#' @export
".rk.killPreviewDevice" <- function (x) {
	a <- .rk.variables$.rk.preview.devices[[x]]
	if (!is.null (a)) {
		if (a$devnum %in% dev.list ()) {
			dev.off (a$devnum)
		}
		.rk.variables$.rk.preview.devices[[x]] <- NULL
	}
}

".rk.discard.preview.device.num" <- function (devnum) {
	for (dev in names (.rk.variables$.rk.preview.devices)) {
		if (.rk.variables$.rk.preview.devices[[dev]]$devnum == devnum) {
			.rk.variables$.rk.preview.devices[[dev]] <- NULL
			return (invisible (TRUE))
		}
	}
	invisible (FALSE)
}

".rk.list.preview.device.numbers" <- function () {
	unlist (sapply (.rk.variables$.rk.preview.devices, function (x) x$devnum))
}

.rk.variables$.rk.printer.devices <- list ()

# see .rk.fix.assignmetns () in internal.R
#' @export
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

			rkward:::.rk.discard.preview.device.num(which)

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
				if (dev.cur() == 1) dev.new ()
				## TODO: use "trellis" instead of "lattice" to accomodate ggplot2 plots?
				plot_hist_enabled <- getOption ("rk.enable.graphics.history")
				if (plot_hist_enabled) {
					rk.record.plot$record (nextplot.pkg = "lattice")
				}
				rk.without.plot.history (plot (x, ...))
				if (plot_hist_enabled) {
					rk.record.plot$.save.tlo.in.hP ()
				}
				invisible ()
			})
	)

	if (compareVersion (as.character (getRversion ()), "2.14.0") < 0) {
		setHook (packageEvent ("grid", "attach"),
			function (...)
				rk.replace.function ("grid.newpage", as.environment ("package:grid"),
					function () {
						## TODO: add specific support for ggplots?
						rk.record.plot$.plot.new.hook ()
						ret <- eval (body (.rk.backups$grid.newpage))
					})
		)
	} else {
		setHook ("before.grid.newpage",
			function (...)
			{
				rk.record.plot$.plot.new.hook ()
			},
			action = "append"
		)
	}

	## persp does not call plot.new (), so set a hook. Fortunately, the hook is placed after drawing the plot.
	setHook ("persp",
		function (...)
		{
			rk.record.plot$.plot.new.hook ()
		},
		action = "append"
	)
}

