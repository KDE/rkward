## Public functions manipulating "graphics" should be stored here.
## These functions are accessible to the user.

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

"rk.graph.off" <- function(){
	.rk.cat.output ("\n")	# so the output will be auto-refreshed
	dev.off()
}

"rk.duplicate.device" <- function (deviceId = dev.cur ())
{
	dev.set (deviceId)
	dev.copy (device = x11)
}

# create a (global) history of various graphics calls - a rudimentary attempt
"rk.record.plot" <- function ()
{
	# TODO: 
	# - add showFirst and showLast to menubar / toolbar
	# - add a length and size limit to recorded () list
	# - add a menu / toolbar to clear history
	# - Create separate history for each device? May be not!
	# - Destroy the history when a device is closed?
	# - .... ?
	
	env <- environment()
	recorded <- list()
	current <- as.list(0) # 1 is always null device
	newPlotExists <- as.list(FALSE)
	
	onAddDevice <- function (duplicateId = 1, deviceId = dev.cur ())
	{
		if (duplicateId > 1) recordUnsaved (duplicateId)
		current [[deviceId]] <<- current [[duplicateId]]
		newPlotExists [[deviceId]] <<- newPlotExists [[duplicateId]]
	}
	onDelDevice <- function (deviceId = dev.cur())
	{
		recordUnsaved (deviceId)
		# using NULL instead of NA, shrinks the list by 1 component, which is exactly the thing to avoid here!
		current [[deviceId]] <<- NA
		newPlotExists [[deviceId]] <<- FALSE
	}
	record <- function(deviceId = dev.cur (), newplotflag = TRUE, force = FALSE)
	{
		cur.deviceId <- dev.cur ()
		dev.set (deviceId)
		if (newPlotExists [[deviceId]]) {
			if (class (try (unsavedPlot <- recordPlot(), silent=TRUE)) != 'try-error') {
				current [[deviceId]] <<- length(recorded) + 1L
				recorded [[current [[deviceId]]]] <<- unsavedPlot
				.rk.graph.history.gui (deviceId)
			}
		} else if (force) {
			if (class (try (unsavedPlot <- recordPlot(), silent=TRUE)) != 'try-error') {
				recorded [[current [[deviceId]]]] <<- unsavedPlot
			}
		}
		newPlotExists [[deviceId]] <<- newplotflag
		dev.set (cur.deviceId)
	}
	recordUnsaved <- function (deviceId = dev.cur ())
	{
		if (newPlotExists [[deviceId]]) {
			record (deviceId, newplotflag = FALSE)
		}
	}
	replay <- function(n = current [[deviceId]] - 1L, deviceId = dev.cur ())
	{
		cur.deviceId <- dev.cur ()
		dev.set (deviceId)
		if (n > 0 && n <= length(recorded)) {
			current [[deviceId]] <<- n
			replayPlot(recorded[[n]])
			.rk.graph.history.gui (deviceId)
		}
		else message("replay: 'n' not in valid range: ", n)
		dev.set (cur.deviceId)
	}
	showFirst <- function(deviceId = dev.cur()) replay(n = 1, deviceId)
	showPrevious <- function(deviceId)
	{
		recordUnsaved (deviceId)
		replay(n = current [[deviceId]] - 1L, deviceId = deviceId)
	}
	showNext <- function(deviceId)
	{
		recordUnsaved (deviceId)
		replay(n = current [[deviceId]] + 1L, deviceId = deviceId)
	}
	showLast <- function(deviceId = dev.cur()) replay(n = length(recorded), deviceId)
	resetHistory <- function ()
	{
		recorded <<- list()
		current <- as.list(0)
		newPlotExists <- as.list(FALSE)
		.rk.graph.history.gui (deviceId)
	}
	printPars <- function ()
	{
		message ('History len: ', length (recorded))
		message ('Current devices: ', paste (unlist (current), collapse = ', ')) 
		message ('New plot exists? ', paste (unlist (newPlotExists), collapse = ', ')) 
	}
	env
}
rk.record.plot <- rk.record.plot ()

# quick wrappers around rk.record.plot$show{Previous,Next} :
# 1 is always the null device
# TODO : comment / remove printPars call
"rk.first.plot" <- function (deviceId = 2)
{
	rk.record.plot$showFirst (deviceId)
	rk.record.plot$printPars ()
}
"rk.previous.plot" <- function (deviceId = 2)
{
	rk.record.plot$showPrevious (deviceId)
	rk.record.plot$printPars ()
}
"rk.next.plot" <- function (deviceId = 2)
{
	rk.record.plot$showNext (deviceId)
	rk.record.plot$printPars ()
}
"rk.last.plot" <- function (deviceId = 2)
{
	rk.record.plot$showLast (deviceId)
	rk.record.plot$printPars ()
}
"rk.current.plot" <- function (deviceId = 2)
{
	if (!(deviceId %in% .rk.preview.devices)) rk.record.plot$record (deviceId, newplotflag=FALSE, force=TRUE)
	rk.record.plot$printPars ()
}
