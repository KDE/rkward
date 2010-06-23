## Internal functions manipulating graphics should be stored here.
## These functions are _not_ supposed to be called by the end user.

# overriding x11 to get informed, when a new x11 window is opened
"rk.screen.device" <- function (...) {
	.rk.do.call ("startOpenX11", as.character (dev.cur ()));

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

"plot.new" <- function () 
{
	rk.record.plot$record ()
	eval (body (.rk.plot.new.default))
}
formals (plot.new) <- formals (graphics::plot.new)
.rk.plot.new.default <- graphics::plot.new

# see .rk.fix.assignmetns () in internal.R
".rk.fix.assignments.graphics" <- function () {
	assignInNamespace ("plot.new", plot.new, envir=as.environment ("package:graphics"))
}
