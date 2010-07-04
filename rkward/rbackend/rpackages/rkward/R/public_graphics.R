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
	rk.record.plot$.set.isDuplicate (TRUE)
	dev.copy (device = x11)
	rk.record.plot$.set.isDuplicate (FALSE)
}

# create a (global) history of various graphics calls - a rudimentary attempt
"rk.record.plot" <- function ()
{
	# TODO: 
	# - add a length and size limit to recorded () list
	# - add option to delete a plot from history
	# - add one or more tests to rkward_application_tests.R
	# - .rk.graph.history.gui () add option to update only one deviceId
	# - .... ?
	
	env <- environment()
	recorded <- list()
	histPositions <- list("1" = 0) # 1 is always null device
	newPlotExists <- list("1" = FALSE)
	isDuplicate <- FALSE
	isPreviewDevice <- FALSE
	
	.set.isDuplicate <- function (x = FALSE) { isDuplicate <<- x }
	.set.isPreviewDevice <- function (x = FALSE) { isPreviewDevice <<- x }
	onAddDevice <- function (old_dev = 1, deviceId = dev.cur ())
	{
		old_dev <- as.character (old_dev)
		deviceId <- as.character (deviceId)
		
		# onAddDevice is called only from rk.screen.device, so no need to check dev.interactive ()
		if (isPreviewDevice) return (invisible (NULL))
		if (old_dev %in% names (histPositions) && old_dev != "1") recordUnsaved (old_dev)
		
		if (isDuplicate) {
			histPositions [[deviceId]] <<- histPositions [[old_dev]]
		} else {
			n <- length (recorded)
			histPositions [[deviceId]] <<- if (n > 0) n + 1 else 0
		}
		newPlotExists [[deviceId]] <<- FALSE
		.rk.graph.history.gui (deviceId)
	}
	onDelDevice <- function (deviceId = dev.cur())
	{
		deviceId <- as.character (deviceId)
		
		if (deviceId %in% names (histPositions) && deviceId != "1") {
			recordUnsaved (deviceId)
			histPositions [[deviceId]] <<- newPlotExists [[deviceId]] <<- NULL
		}
	}
	record <- function(deviceId = dev.cur (), newplotflag = TRUE, force = FALSE)
	{
		deviceId <- as.character (deviceId)
		
		isManaged <- deviceId %in% names (histPositions)
		if (!isManaged && !force) return (invisible (NULL)) # --- (*)
		
		cur.deviceId <- dev.cur ()
		dev.set (as.numeric(deviceId))
		
		if (isManaged) {
			# device is managed, that is, non-preview-interactive
			
			if (newPlotExists [[deviceId]]) {
				# there is a new plot on this device, so save it,
				# immaterial of whether force == TRUE or FALSE
				
				if (class (try (unsavedPlot <- recordPlot(), silent=TRUE)) != 'try-error') {
					histPositions [[deviceId]] <<- n <- length(recorded) + 1
					recorded [[n]] <<- unsavedPlot
					.rk.graph.history.gui ()
				}
			} else if (force) {
				# no new plot on this managed device but force == TRUE
				# in other words, called from a non-preview interactive device by clicking "Add to history" icon
				# so overwrite the existing plot in history by the current plot
				# 
				# use case:
				# go back in history and update the plot using points () or lines () or ...
				
				n <- histPositions [[deviceId]]
				if (n == 0) {
					newPlotExists [[deviceId]] <<- TRUE
					record (deviceId, newplotflag = FALSE, force = FALSE)
				} else {
					if (class (try (unsavedPlot <- recordPlot(), silent=TRUE)) != 'try-error') {
						recorded [[n]] <<- unsavedPlot
					}
				}
			}
			newPlotExists [[deviceId]] <<- newplotflag
		} else {
			# device is not managed but due to (*) force == TRUE
			# in other words, called from a preview device by clicking "Add to history" icon
			# note: non-interactive devices such as pdf() png() etc. get returned at (*)
			#
			# use case:
			# save a particular "preview" plot to history (useful since preview plots are _not_
			# automatically added to history)
			
			n <- length (recorded) + 1
			if (class (try (unsavedPlot <- recordPlot(), silent=TRUE)) != 'try-error') {
				recorded [[n]] <<- unsavedPlot
				.rk.graph.history.gui ()
			}
		}
		
		dev.set (cur.deviceId)
	}
	recordUnsaved <- function (deviceId = dev.cur ())
	{
		if (newPlotExists [[as.character (deviceId)]]) {
			record (deviceId, newplotflag = FALSE)
		}
	}
	replay <- function(n = histPositions [[as.character (deviceId)]] - 1L, deviceId = dev.cur ())
	{
		deviceId <- as.character (deviceId)
		
		cur.deviceId <- dev.cur ()
		dev.set (as.numeric(deviceId))
		
		if (n > 0 && n <= length(recorded)) {
			histPositions [[deviceId]] <<- n
			replayPlot(recorded[[n]])
			.rk.graph.history.gui (deviceId)
		}
		else message("replay: 'n' not in valid range: ", n)
		dev.set (cur.deviceId)
	}
	showFirst <- function(deviceId = dev.cur())
	{
		recordUnsaved (deviceId)
		replay(n = 1, deviceId)
	}
	showPrevious <- function(deviceId)
	{
		recordUnsaved (deviceId)
		replay(n = histPositions [[as.character (deviceId)]] - 1L, deviceId = deviceId)
	}
	showNext <- function(deviceId)
	{
		recordUnsaved (deviceId)
		replay(n = histPositions [[as.character (deviceId)]] + 1L, deviceId = deviceId)
	}
	showLast <- function(deviceId = dev.cur())
	{
		recordUnsaved (deviceId)
		replay(n = length(recorded), deviceId)
	}
	clearHistory <- function ()
	{
		recorded <<- list()
		isDuplicate <<- FALSE
		isPreviewDevice <<- FALSE
		for (dev_num in names (histPositions)) {
			histPositions[[dev_num]] <<- 0
			newPlotExists [[dev_num]] <<- FALSE
		}
		.rk.graph.history.gui ()
	}
	printPars <- function ()
	{
		message ('History len: ', length (recorded))
		message ('Current devices  : ', paste (names (histPositions), collapse = ', ')) 
		message ('Current positions: ', paste (unlist (histPositions), collapse = ', ')) 
		message ('New plot exists? ', paste (unlist (newPlotExists), collapse = ', ')) 
	}
	.rk.graph.history.gui <- function (deviceId = NULL)
	{
		# this function is called whenever the history length changes (ie, increases, for now)
		# or the position changes in any device.
		history_length <- length (recorded)
		
		if (is.null (deviceId)) {
			# update all managed devices:
			ndevs <- length (histPositions)
			if (ndevs > 1) {
				positions <- character (1 + 2 * ndevs)
				positions [1] <- history_length # coerced as character
				positions [2 * (1:ndevs)] <- names (histPositions)
				positions [1 + 2 * (1:ndevs)] <- unlist (histPositions, use.names = FALSE)
			}
		} else {
			# update only the one device: deviceId:
			positions <- c(history_length, deviceId, histPositions [[as.character (deviceId)]])
			positions <- as.character (positions)
			names (positions) <- NULL
		}
		
		.rk.do.call ("updateDeviceHistory", positions);
		invisible (NULL)
	}

	env
}
rk.record.plot <- rk.record.plot ()

# quick wrappers around rk.record.plot$show{Previous,Next} :
# 1 is always the null device
# TODO : comment / remove printPars call
"rk.first.plot" <- function (deviceId = dev.cur ())
{
	rk.record.plot$showFirst (deviceId)
	rk.record.plot$printPars ()
}
"rk.previous.plot" <- function (deviceId = dev.cur ())
{
	rk.record.plot$showPrevious (deviceId)
	rk.record.plot$printPars ()
}
"rk.next.plot" <- function (deviceId = dev.cur ())
{
	rk.record.plot$showNext (deviceId)
	rk.record.plot$printPars ()
}
"rk.last.plot" <- function (deviceId = dev.cur ())
{
	rk.record.plot$showLast (deviceId)
	rk.record.plot$printPars ()
}
"rk.addthis.plot" <- function (deviceId = dev.cur ())
{
	# this call is not as simple as it looks; details are handled inside rk.record.plot$record ()
	# 
	# reason:
	# flixibility to add a preview plot (preview device is _not_ managed) to the graphics history
	
	rk.record.plot$record (deviceId, newplotflag=FALSE, force=TRUE)
	rk.record.plot$printPars ()
}
