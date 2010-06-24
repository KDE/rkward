## Internal functions manipulating graphics should be stored here.
## These functions are _not_ supposed to be called by the end user.

# overriding x11 to get informed, when a new x11 window is opened
"rk.screen.device" <- function (..., is.preview.device = FALSE) {
	.rk.do.call ("startOpenX11", as.character (dev.cur ()));

	if (!is.preview.device) dupFrom <- dev.cur ()

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

	if (!is.preview.device) rk.record.plot$onAddDevice (duplicateId = dupFrom, deviceId = dev.cur ())

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

".rk.graph.history.gui" <- function (deviceId = dev.cur())
{
	# this function is called whenever the history length changes (ie, increases, for now)
	# or the position changes in any device.
	# see public_graphics.R :: rk.record.plot
	invisible ()
}

"plot.new" <- function () 
{
	if (dev.cur() == 1) rk.screen.device ()
	if (dev.interactive () && !(dev.cur() %in% .rk.preview.devices)) rk.record.plot$record ()
	eval (body (.rk.plot.new.default))
}
formals (plot.new) <- formals (graphics::plot.new)
.rk.plot.new.default <- graphics::plot.new

"dev.off" <- function (which = dev.cur ())
{
	.is.inter <- dev.interactive ()
	if (.is.inter) { 
		# Why use 'which'? There is a which ()!!
		if (!(which %in% .rk.preview.devices)) rk.record.plot$onDelDevice (deviceId = which)
	}
	
	eval (body (.rk.dev.off.default))
	.ret.value <- .Last.value
	return (.ret.value)
}
formals (dev.off) <- formals (grDevices::dev.off)
.rk.dev.off.default <- grDevices::dev.off

# see .rk.fix.assignmetns () in internal.R
".rk.fix.assignments.graphics" <- function ()
{
	assignInNamespace ("plot.new", plot.new, envir=as.environment ("package:graphics"))
	assignInNamespace ("dev.off", dev.off, envir=as.environment ("package:grDevices"))
}
