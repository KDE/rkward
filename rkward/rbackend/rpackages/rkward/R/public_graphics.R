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

"rk.activate.device" <- function (deviceId = dev.cur ())
{
	dev.set (deviceId)
	rk.record.plot$.set.trellis.last.object (deviceId)
}

# A global history of various graphics calls; trellis / grid graphics is not supported yet
"rk.record.plot" <- function ()
{
	# TODO: 
	# - check when decreasing the max history length below the current recorded length
	# - add one or more tests to rkward_application_tests.R
	# - .... ?
	
	env <- environment()
	recorded <- list()
	histPositions <- list("1" = 0) # 1 is always null device
	newPlotExists <- list("1" = FALSE)
	isDuplicate <- FALSE
	isPreviewDevice <- FALSE
	gType <- list ()
	gType.newplot <- ""
	
	.set.isDuplicate <- function (x = FALSE) { isDuplicate <<- x }
	.set.isPreviewDevice <- function (x = FALSE) { isPreviewDevice <<- x }
	.set.gType.newplot <- function (x) gType.newplot <<- x
	.set.trellis.last.object <- function (deviceId = dev.cur ())
	{
		deviceId <- as.character (deviceId)
		if (gType[[histPositions [[deviceId]]]] == "lattice")
			assign ("last.object", recorded[[histPositions [[deviceId]]]], envir = lattice:::.LatticeEnv)
		invisible ()
	}
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
		.rk.graph.history.gui () # (deviceId)
	}
	onDelDevice <- function (deviceId = dev.cur())
	{
		deviceId <- as.character (deviceId)
		
		if (deviceId %in% names (histPositions) && deviceId != "1") {
			recordUnsaved (deviceId)
			histPositions [[deviceId]] <<- newPlotExists [[deviceId]] <<- NULL
		}
	}
	push.pop.and.record <- function (which.pop = NULL, which.push = NULL, deviceId = NULL, newplot = FALSE, oldplot = !newplot)
	{
		unsavedPlot <- NULL
		actually.record.the.plot <- function ()
		{
			retval <- FALSE
			if (gType.newplot == "standard") {
				if (class (try (unsavedPlot <<- recordPlot(), silent=TRUE)) != "try-error") retval <- TRUE
			} else if  (gType.newplot == "lattice") {
				if (class (try (unsavedPlot <<- trellis.last.object (), silent=TRUE)) != "try-error") retval <- TRUE
			}
			return (retval)
		}
		if (actually.record.the.plot ()) {
			s <- object.size (unsavedPlot) # in bytes
			
			if (s <= getOption ('rk.graphics.hist.max.plotsize') * 1024) {
				if (oldplot) {
					recorded [[which.push]] <<- unsavedPlot
					gType [[which.push]] <<- gType.newplot
					return (TRUE)
				}
				
				len.r <- length(recorded)
				ml <- getOption ('rk.graphics.hist.max.length')
				
				if (len.r < ml) {
					n <- len.r + 1
				} else if (len.r == ml) {
					warning ('Max length reached, popping out the first plot.')
					remove (deviceId = NULL, pos = which.pop)
					n <- len.r
				} else {
					warning ('Current history length > max length: plot not added to history!')
					return (FALSE)
				}
				
				if (!is.null (deviceId)) histPositions [[deviceId]] <<- n
				.rk.graph.history.gui ()
				recorded [[n]] <<- unsavedPlot
				gType [[n]] <<- gType.newplot
				
				return (TRUE)
			} else {
				warning ('Oversized plot: not added to history!') # don't use stop (...)
				return (FALSE)
			}
		} else {
			warning ('Unable to record the plot!') # don't use stop (...)
			return (FALSE)
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
			
			succeded <- TRUE
			if (newPlotExists [[deviceId]]) {
				# there is a new plot on this device, so save it,
				# immaterial of whether force == TRUE or FALSE
				
				succeded <- push.pop.and.record (which.pop = 1, deviceId = deviceId, newplot = TRUE)
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
					succeded <- push.pop.and.record (which.push = n, oldplot = TRUE)
				}
			}
			if (succeded || !force)
				newPlotExists [[deviceId]] <<- newplotflag
		} else {
			# device is not managed but due to (*) force == TRUE
			# in other words, called from a preview device by clicking "Add to history" icon
			# note: non-interactive devices such as pdf() png() etc. get returned at (*)
			#
			# use case:
			# save a particular "preview" plot to history (useful since preview plots are _not_
			# automatically added to history)
			
			push.pop.and.record (which.pop = 1, deviceId = NULL, newplot = TRUE)
		}
		
		dev.set (cur.deviceId)
	}
	recordUnsaved <- function (deviceId = dev.cur ())
	{
		if (newPlotExists [[as.character (deviceId)]]) {
			record (deviceId, newplotflag = FALSE)
		}
	}
	remove <- function (deviceId = dev.cur (), pos = NULL) # pos can be of length > 1
	{
		history_length <- length (recorded)
		if (history_length <= 1) {
			if (history_length == 1) .rk.graph.history.gui ()
			return (invisible (NULL))
		}
		
		pop.and.update <- function (n) {
			## TODO: check if this is too expensive? Use recorded[[n]] <<- NULL ??
			recorded <<- recorded [-n]
			gType <<- gType [-n]
			len.r <- length (recorded)
			
			pos.aff <- unlist (histPositions) >= min (n) # all affected positions
			pos.rem <- unlist (histPositions) %in% n # only removed positions
			
			dEqn <- names (histPositions)[pos.rem] # devices whose plots were removed
			for (d in dEqn) {
				m <- min (histPositions[[d]] - sum (n <= histPositions[[d]]) + 1, len.r)
				if (newPlotExists[[d]]) {
					histPositions [[d]] <<- m
					#.rk.graph.history.gui () # (d)
				} else
					replay (n = m, deviceId = d)
			}
			
			dGtn <- names (histPositions)[pos.aff & !pos.rem] # affected devices whose plots were _NOT_ removed
			for (d in dGtn) {
				histPositions[[d]] <<- histPositions[[d]] - sum (n <= histPositions[[d]])
			}
			.rk.graph.history.gui () # (dGtn)
		}
		
		if (is.null (pos)) {
			# call from: a managed device by clicking on 'Remove from history' icon
			
			if (is.null (deviceId)) stop ('Both deviceId and pos are NULL')
			deviceId <- as.character (deviceId)
			if (! (deviceId %in% names(histPositions))) stop (paste ('Device', deviceId, 'is not managed'))
			
			pos <- histPositions [[deviceId]]
			
			if (newPlotExists [[deviceId]]) {
				# current plot, which is to be deleted, hasn't been saved to history yet, so just 
				# set its flag to FALSE and replay the previous plot which is @ pos and not (pos-1)
				
				newPlotExists [[deviceId]] <<- FALSE
				replay (n = pos, deviceId)
			} else {
				# current plot is a saved plot: so pop it and update the "affected" devices
				
				pop.and.update (n = pos)
			}
		} else if (all(pos > 0) && all (pos <= history_length)) {
			# call from: push.pop.and.record () (see above) not from any device
			
			pop.and.update (n = pos)
		} else
			stop (paste ('Invalid position(s)'))
		
		invisible (NULL)
	}
	replay <- function(n = histPositions [[as.character (deviceId)]] - 1L, deviceId = dev.cur ())
	{
		deviceId <- as.character (deviceId)
		
		cur.deviceId <- dev.cur ()
		dev.set (as.numeric(deviceId))
		
		if (n > 0 && n <= length(recorded)) {
			if (gType [[n]] == "standard") {
				replayPlot (recorded[[n]])
			} else if (gType [[n]] == "lattice") {
				message (deparse (recorded[[n]]$call)) # show case call object
				plot (recorded[[n]], save.object = (cur.deviceId == as.numeric (deviceId)))
			}
			histPositions [[deviceId]] <<- n
			.set.gType.newplot (gType [[n]])
			.rk.graph.history.gui () # (deviceId)
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
		gType <<- list ()
		gType.newplot <<- ""
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
		message ('gType @ these pos: ', paste (unlist (gType [unlist (histPositions)]), collapse = ', '))
		message ('gType newplot?   : ', gType.newplot)
	}
	.rk.graph.history.gui <- function (deviceIds = names (histPositions))
	{
		# this function is called whenever the history length changes
		# or the position changes in any device.
		
		deviceIds <- deviceIds [deviceIds != "1"] # ignore NULL device
		ndevs <- length (deviceIds)
		if (ndevs>0) {
			positions <- character (1 + 2 * ndevs)
			positions [1] <- length (recorded) # coerced as character
			positions [2 * (1:ndevs)] <- deviceIds
			positions [1 + 2 * (1:ndevs)] <- unlist (histPositions[deviceIds], use.names = FALSE)
			.rk.do.call ("updateDeviceHistory", positions);
		}
		invisible (NULL)
	}
	.verify.hist.limits <- function ()
	{
		# Length restriction:
		len.max <- getOption ('rk.graphics.hist.max.length')
		len.r <- length (recorded)
		
		ans <- 'no'
		if (len.max < len.r) {
			## TODO: implement using rk.ask.yesnocancel ()
			ans <- readline (paste ('Current screen history has more plots than the maximum number chosen. ',len.r - len.max,' of the foremost plots will be removed.\nIf you want to continue type [y]es. Instead, if you prefer to remove them yourself type [n]o or hit Cancel.', sep = ''))
			if (tolower(ans) %in% c('y', 'yes'))
				remove (deviceId = NULL, pos = 1:(len.r - len.max))
		}
		
		# Size restriction:
		#s <- getOption ('rk.graphics.hist.max.plotsize')
		# Existing plots are not checked for their sizes, only the new ones are.
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
"rk.removethis.plot" <- function (deviceId = dev.cur ())
{
	rk.record.plot$remove (deviceId)
	rk.record.plot$printPars ()
}
