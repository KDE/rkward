## Public functions manipulating "graphics" should be stored here.
## These functions are accessible to the user.

# Requests a graph to be written.
rk.graph.on <- function (device.type=getOption ("rk.graphics.type"), width=getOption ("rk.graphics.width"), height=getOption ("rk.graphics.height"), quality, ..., set.active.device = TRUE) 
{
	if (!is.numeric (width)) width <- 480
	if (!is.numeric (height)) height <- 480
	if (is.null (device.type)) device.type <- "PNG"	# default behavior is PNG for now

	if (set.active.device) assign (".rk.active.device", dev.cur (), pos = "package:rkward")

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
	ret <- dev.off()
	
	# dev.off () sets dev.next () as active, which may not have been active before rk.graph.on was called; so reset the correct device as active:
	i <- get (".rk.active.device", pos = "package:rkward")
	if ((!is.null (i)) && i > 1) dev.set (i)
	ret
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
	gType.newplot <- list ()
	
	.set.isDuplicate <- function (x = FALSE) { isDuplicate <<- x }
	.set.isPreviewDevice <- function (x = FALSE) { isPreviewDevice <<- x }
	.set.trellis.last.object <- function (deviceId = dev.cur ())
	{
		deviceId <- as.character (deviceId)
		if (gType[[histPositions [[deviceId]]]] == "lattice")
			assign ("last.object", recorded[[histPositions [[deviceId]]]], envir = lattice:::.LatticeEnv)
		invisible ()
	}
	onAddDevice <- function (old_dev = 1, deviceId = dev.cur ())
	{
		# onAddDevice is called only from rk.screen.device, so no need to check dev.interactive ()
		
		old_dev <- as.character (old_dev)
		deviceId <- as.character (deviceId)
		
		if (isPreviewDevice) return (invisible (NULL))
		
		# save any unsaved plots before duplicating:
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
		
		# save any unsaved plot before closing the device / window
		if (deviceId %in% names (histPositions) && deviceId != "1") {
			recordUnsaved (deviceId)
			histPositions [[deviceId]] <<- newPlotExists [[deviceId]] <<- NULL
		}
	}
## TODO: newplot -> this.plot.is.new, remove oldplot
	push.pop.and.record <- function (which.pop = NULL, which.push = NULL, deviceId = NULL, newplot = FALSE, oldplot = !newplot)
	{
		actually.record.the.plot <- function ()
		{
			# function defined w/o arguments bcoz, "this.plot.gType" is used in multiple places
			# so why not use it here as well...
			retval <- FALSE
			if (this.plot.gType == "standard") {
				if (class (try (unsavedPlot <<- recordPlot(), silent=TRUE)) != "try-error") retval <- TRUE
			} else if  (this.plot.gType == "lattice") {
				if (class (try (unsavedPlot <<- trellis.last.object (), silent=TRUE)) != "try-error") retval <- TRUE
			}
			return (retval)
		}
		
		unsavedPlot <- NULL
		this.plot.gType <- ""
		recording.succeeded <- FALSE
		
## TODO: add comments for each sub-block
		if (is.null (deviceId)) {
			this.plot.gType <- "standard"
			recording.succeeded <- actually.record.the.plot ()
		} else if (newplot) {
			this.plot.gType <- gType.newplot [[deviceId]]
			recording.succeeded <- actually.record.the.plot ()
		} else {
			# see "oldplot = TRUE" block below:
			this.plot.gType <- gType [[histPositions [[deviceId]]]]
			recording.succeeded <- actually.record.the.plot ()
		}
		
		if (recording.succeeded) {
			s <- object.size (unsavedPlot) # in bytes
			
			if (s <= getOption ('rk.graphics.hist.max.plotsize') * 1024) {
				if (oldplot) {
					# One can not overwrite / replace-in-position an existing plot by a completely new plot...
					#   thus, no change to gType.newplot [[]].
					# When recording over an existing plot, the graphics type must remain same... 
					#   thus, no change to gType [[]].
					# See the "force = TRUE" block of record () function for further details
					recorded [[which.push]] <<- unsavedPlot
					
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
				recorded [[n]] <<- unsavedPlot
				gType [[n]] <<- this.plot.gType
				.rk.graph.history.gui ()
				
## TODO: update comment
				# after a successful recording, remove ....
				if (!is.null (deviceId)) gType.newplot [[deviceId]] <<- NULL
				
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
## TODO: newplotflag -> newplot.in.queue
	record <- function(deviceId = dev.cur (), newplotflag = TRUE, force = FALSE, newplot.gType = '')
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
				# go back/forward in history and update the plot using points () or lines () or ...
				# 
				## TODO:
				# does not apply to trellis plots since any update using the "update (...)" call,
				# in turn, calls print.trellis (...) which creates a new plot... would like to rectify this
				# someday! of course, assignments calls, "update<- ", suppresses print.trellis!
				
				n <- histPositions [[deviceId]]
				if (n == 0) {
					# This case arises when the user clears the history, while multiple screen devices are still open...
					# The "Add to history" icon is active on all these open devices and the user can choose
					#   to add the displayed plots to the (now, prestine) history. Hence, this block.
					# See the comments in clearHistory () for further details.
					
					newPlotExists [[deviceId]] <<- TRUE
					record (deviceId, newplotflag = FALSE, force = FALSE)
				} else {
					succeded <- push.pop.and.record (which.push = n, deviceId = deviceId, oldplot = TRUE)
				}
			}
			if (succeded || !force) {
## TODO: update comment
				# when not "force"d, if for some reason, recording did not succeed, do not alter the
				# status (whether or not new plot exists) of the current device
				newPlotExists [[deviceId]] <<- newplotflag
				if (newplotflag) gType.newplot [[deviceId]] <<- newplot.gType
			}
		} else {
			# device is not managed but due to (*) force == TRUE
			# in other words, called from a preview device by clicking "Add to history" icon
			# note: non-interactive devices such as pdf() png() etc. get returned at (*)
			#
			# use case:
			# save a particular "preview" plot to history (useful since preview plots are _not_
			# automatically added to history)
			# 
			# in such a case, gType.newplot [[deviceId]] is non-existant
			
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
			# length (n) can be > 1: see .verify.hist.limits ()
## TODO:
			# split n = 1 (commonly used) and n > 1 cases (only from .verify.hist.limits) to improve performance??
			
			## TODO: investigate b/n x <<- x[-n] & x[n] <<- NULL
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
## TODO: update comment
			# pos == NULL means that ...
			# call from: a managed device by clicking on 'Remove from history' icon
			
			if (is.null (deviceId)) stop ('Both deviceId and pos are NULL') # why should this happen ??
			deviceId <- as.character (deviceId)
			if (! (deviceId %in% names(histPositions))) stop (paste ('Device', deviceId, 'is not managed'))
			
			pos <- histPositions [[deviceId]] # here length (pos) = 1
			
			if (newPlotExists [[deviceId]]) {
				# current plot, which is to be deleted, hasn't been saved to history yet, so just 
				# set its flag to FALSE, remove corresponding gType.newplot entry and
				# replay the previous plot which is @ pos and not (pos-1)
				
				newPlotExists [[deviceId]] <<- FALSE
				gType.newplot [[deviceId]] <<- NULL
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
## TODO: update comment?
		# when this function is called, there are NO unsaved plots! Saving the unsaved plot is taken care off
		# by the wrapper functions, showXxxxx (), below
		
		deviceId <- as.character (deviceId)
		
		cur.deviceId <- dev.cur ()
		dev.set (as.numeric(deviceId))
		
		status.display <- paste ("Dev: ", deviceId, ", Pos: ", n, sep = '')
		if (n > 0 && n <= length(recorded)) {
			if (gType [[n]] == "standard") {
				status.display <- paste (status.display, ", Call: Standard graphics", sep = "")
				replayPlot (recorded[[n]])
			} else if (gType [[n]] == "lattice") {
				status.display <- paste (status.display, ", Call: ", deparse (recorded[[n]]$call), sep = "")
				plot (recorded[[n]], save.object = (cur.deviceId == as.numeric (deviceId)))
			}
			status.display <- paste (status.display, ", Size: ", round (object.size (recorded[[n]])/1024, 2), " Kb", sep = "")
			message ("--\n", status.display, "\n--\n") # add to a new status bar?
			histPositions [[deviceId]] <<- n
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
		
## TODO: update comment:
		# although clear history is clicked, the "+" icon is active and the displayed plot shuold be recorded
		
		for (dev_num in names (histPositions)[-1]) {
			# if the displayed plot is not new, save its type from gType, else leave gType.newplot unchaged
			# IMP: this part has to come before resetting histPositions and newPlotExists.
			if (!newPlotExists [[dev_num]])
				gType.newplot [[dev_num]] <<- gType [[histPositions[[dev_num]]]]
			
			histPositions[[dev_num]] <<- 0
			newPlotExists [[dev_num]] <<- FALSE
		}
		gType <<- list () # IMP: reset gType only AFTER the for loop
		# DO NOT reset gType.newplot list at all
		.rk.graph.history.gui ()
	}
	printPars <- function ()
	{
		message ('History length   : ', length (recorded))
		message ("History size (KB): ", round (object.size (recorded) / 1024, 2))
		message ('Current devices  : ', paste (names (histPositions), collapse = ', ')) 
		message ('Current positions: ', paste (unlist (histPositions), collapse = ', ')) 
		message ('New plot exists? : ', paste (unlist (newPlotExists), collapse = ', ')) 
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
