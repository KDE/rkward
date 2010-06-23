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

# create a (global) history of various graphics calls - a rudimentary attempt
# can do: record, showPrevious, showNext, replay
"rk.record.plot" <- function ()
{
	# TODO: 
	# - record / show from which device? - Partially implemented
	# - Create separate history for each device?
	# - Destroy the history when a device is closed?
	# - .... ?
	
	env <- environment()
	recorded <- list()
	current <- numeric (length(dev.list()) + 2); # 1 is always null device
	newPlotExists <- FALSE
	
	onAddDevice <- function (deviceId)
	{
		recordUnsaved (deviceId)
		current <<- c(current, current[deviceId])
	}
	onDelDevice <- function (deviceId = dev.cur())
	{
		recordUnsaved (deviceId)
		current <<- current[-deviceId]
	}
	record <- function(newplotflag = TRUE, force = FALSE)
	{
		if (newPlotExists) {
			if (class (try (unsavedPlot <- recordPlot(), silent=TRUE)) != 'try-error')
			{
				current[dev.cur()] <<- length(recorded) + 1L
				recorded[[current[dev.cur()]]] <<- unsavedPlot
			}
		}
		if (force) {
			if (class (try (unsavedPlot <- recordPlot(), silent=TRUE)) != 'try-error')
			{
				recorded[[current[dev.cur()]]] <<- unsavedPlot
			}
		}
		newPlotExists <<- newplotflag
	}
	recordUnsaved <- function (deviceId)
	{
		if ((current[deviceId] == length (recorded)) && newPlotExists) {
			record (newplotflag = FALSE)
		}
	}
	replay <- function(n = current[dev.cur()] - 1L, deviceId = dev.cur ())
	{
		if (n > 0 && n <= length(recorded)) {
			current[deviceId] <<- n
			replayPlot(recorded[[n]])
		}
		#else message("'n' not in valid range: ", n)
	}
	restore <- function() replay(n = length(recorded))
	showPrevious <- function(deviceId)
	{
		recordUnsaved (deviceId)
		replay(n = current[deviceId] - 1L, deviceId = deviceId)
	}
	showNext <- function(deviceId)
	{
		recordUnsaved (deviceId)
		replay(n = current[deviceId] + 1L, deviceId = deviceId)
	}
	resetHistory <- function ()
	{
		recorded <<- list()
		current <<- numeric (length(dev.list()) + 2)
		newPlotExists <<- FALSE
		
	}
	env
}
rk.record.plot <- rk.record.plot ()

# quick wrappers around rk.record.plot$show{Previous,Next} :
# 1 is always the null device
"rk.next.plot" <- function (deviceId = 2)
{
	# TODO - utilze the device number when rk.record.plot matures
	cur.deviceId <- dev.cur ()
	dev.set (which = deviceId)
	rk.record.plot$showNext (deviceId)
	dev.set (which = cur.deviceId)
	invisible ()
}
"rk.current.plot" <- function (deviceId = 2)
{
	# TODO - utilze the device number when rk.record.plot matures
	cur.deviceId <- dev.cur ()
	dev.set (which = deviceId)
	rk.record.plot$record (newplotflag=FALSE, force=TRUE)
	dev.set (which = cur.deviceId)
	invisible ()
}
"rk.previous.plot" <- function (deviceId = 2)
{
	# TODO - utilze the device number when rk.record.plot matures
	cur.deviceId <- dev.cur ()
	dev.set (which = deviceId)
	rk.record.plot$showPrevious (deviceId)
	dev.set (which = cur.deviceId)
	invisible ()
}

