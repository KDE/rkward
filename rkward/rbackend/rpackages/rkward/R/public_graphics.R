## Public functions manipulating "graphics" should be stored here.
## These functions are accessible to the user.

# Requests a graph to be written.
rk.graph.on <- function (device.type=getOption ("rk.graphics.type"), width=getOption ("rk.graphics.width"), height=getOption ("rk.graphics.height"), quality, ...) 
{
	if (!is.numeric (width)) width <- 480
	if (!is.numeric (height)) height <- 480
	if (is.null (device.type)) device.type <- "PNG"	# default behavior is PNG for now

	assign (".rk.active.device", dev.cur (), pos = "package:rkward")

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
	
	# dev.off () sets dev.next () as active, which may not have been active before rk.graph.on was called;
	# so reset the correct device as active:
	i <- get (".rk.active.device", pos = "package:rkward")
	if ((!is.null (i)) && (i %in% dev.list ())) dev.set (i)
	ret
}

"rk.duplicate.device" <- function (deviceId = dev.cur ())
{
	dev.set (deviceId)
	rk.record.plot$.set.isDuplicate (TRUE)
	dev.copy (device = x11)
	rk.record.plot$.set.isDuplicate (FALSE)
	#rk.record.plot$printPars () # DEBUG
}

"rk.activate.device" <- function (deviceId = dev.cur ())
{
	dev.set (deviceId)
	rk.record.plot$.set.trellis.last.object (deviceId)
	#rk.record.plot$printPars () # DEBUG
}

# A global history of various graphics calls;
"rk.record.plot" <- function ()
{
## TODO: 
	# - add one or more tests to rkward_application_tests.R
	# - .... ?
	
	env <- environment()
	recorded <- list()
	histPositions <- list("1" = 0)     # one element for every managed graphics device / window; 1 is always null device
	replacePositions <- list ("1" = 0)
	isDuplicate <- FALSE
	isPreviewDevice <- FALSE
	
	# graphics types (standard / lattice / ...) for the stored / new plots
	gType <- list ()
	
	.set.isDuplicate <- function (x = FALSE) { isDuplicate <<- x }
	.set.isPreviewDevice <- function (x = FALSE) { isPreviewDevice <<- x }
	.set.trellis.last.object <- function (deviceId = dev.cur ())
	{
		deviceId <- as.character (deviceId)
		n <- histPositions [[deviceId]]
		gType.n.exists <- length (gType) >= n
		recorded.n.exists <- length (recorded) >= n
		if (n > 0 && gType.n.exists && recorded.n.exists && gType[[n]] == "lattice")
			assign ("last.object", recorded[[n]], envir = lattice:::.LatticeEnv)
		invisible ()
	}
	onAddDevice <- function (old_dev = 1, deviceId = dev.cur ())
	{
		# onAddDevice is called only from rk.screen.device, so no need to check dev.interactive ()
		
		if (isPreviewDevice) return (invisible (NULL))
		
		old_dev <- as.character (old_dev)
		deviceId <- as.character (deviceId)
		
		# save any unsaved plots before duplicating:
		if ((old_dev %in% names (histPositions)) && (old_dev != "1") && (histPositions[[old_dev]] > 0))
			record (old_dev)
		
		if (isDuplicate) {
			histPositions [[deviceId]] <<- histPositions [[old_dev]]
			replacePositions [[deviceId]] <<- replacePositions [[old_dev]]
		} else {
			n <- length (recorded)
			histPositions [[deviceId]] <<- if (n > 0) n+1 else 0
			replacePositions [[deviceId]] <<- 0
		}
		.rk.graph.history.gui () # (deviceId)
	}
	onDelDevice <- function (deviceId = dev.cur())
	{
		deviceId <- as.character (deviceId)
		
		# save any unsaved plot before closing the device / window
		if (deviceId %in% names (histPositions) && deviceId != "1"  && histPositions[[deviceId]] > 0) {
			record (deviceId)
			histPositions [[deviceId]] <<- NULL
			replacePositions [[deviceId]] <<- NULL
		}
		#printPars () # DEBUG
	}
	.grow.history <- function (deviceId, np.gT)
	{
		len.r <- length(recorded)
		ml <- getOption ('rk.graphics.hist.max.length')
		
		if (len.r < ml) {
			n <- len.r + 1
		} else if (len.r == ml) {
			warning ('Max length reached, popping out the first plot.')
			remove (deviceId = NULL, pos = 1)
			n <- len.r
		} else {
			warning ('Current history length > max length: plot not added to history!')
			return (invisible ())
		}
		replacePositions [[deviceId]] <<- histPositions [[deviceId]]
		histPositions [[deviceId]] <<- n
		gType [[n]] <<- np.gT
		invisible ()
	}
	record.all.recordable <- function ()
	{
		for (d in names(histPositions)[-1]) {
			n <- histPositions[[d]]
			gType.n.exists <- length (gType) >= n
			if (n > 0 && gType.n.exists) record (d)
		}
		invisible ()
	}
	record <- function(deviceId = dev.cur (), newplot.gType = NULL)
	{
		deviceId <- as.character (deviceId)
		
		isManaged <- deviceId %in% names (histPositions)
		
		# non-interactive devices, such as pdf (), png (), ... are returned at this stage:
		if (!isManaged) return (invisible (NULL)) # --- (*)
		
		if (isManaged) {
			# device is managed, that is, non-preview-interactive
			
			cur.deviceId <- dev.cur ()
			dev.set (as.numeric(deviceId))
		
			if (histPositions [[deviceId]] == 0) .grow.history (deviceId, NULL)
			n <- histPositions [[deviceId]]
			unsavedPlot <- NULL
			recording.succeeded <- FALSE
			gType.n.exists <- length (gType) >= n
			recorded.n.exists <- length (recorded) >= n
			
			if (gType.n.exists) {
				if (gType[[n]] == "standard") {
					if (class (try (unsavedPlot <- recordPlot(), silent=TRUE)) != "try-error") recording.succeeded <- TRUE
				} else if  (gType[[n]] == "lattice") {
					if (class (try (unsavedPlot <- trellis.last.object (), silent=TRUE)) != "try-error") recording.succeeded <- TRUE
				}
			}
			
			if (recording.succeeded) {
				s <- object.size (unsavedPlot) # in bytes
				
				if (s <= getOption ('rk.graphics.hist.max.plotsize') * 1024) {
					recorded [[n]] <<- unsavedPlot
					if (!is.null (newplot.gType)) {
						.grow.history (deviceId, newplot.gType)
					} else {
						replacePositions [[deviceId]] <<- n
					}
				} else {
					# this oversized plot is lost :(
					warning ('Oversized plot: not added to history!') # don't use stop (...)
					if ((!is.null (newplot.gType)) && !recorded.n.exists) gType [[n]] <<- newplot.gType
				}
			} else {
				if (gType.n.exists) warning ("Recording failed for some reason.")
				if ((!is.null (newplot.gType)) && !recorded.n.exists) gType [[n]] <<- newplot.gType
			}
			
			dev.set (cur.deviceId)
			.rk.graph.history.gui ()
			#printPars () # DEBUG
			return (invisible ())
		}
	}
	remove <- function (deviceId = dev.cur (), pos = NULL) # pos can be of length > 1
	{
		history_length <- length (recorded)
		if (history_length == 1) {
			clearHistory ()
			rk.show.message ("Plot history cleared!")
		}
		if (history_length <= 1) {
			return (invisible (NULL))
		}
		
		pop.and.update <- function (n) {
			# length (n) can be > 1: see .verify.hist.limits ()
			
			len.n <- length (n)
			recorded[n] <<- NULL
			gType[n] <<- NULL
			len.r <- length (recorded)
			
			#printPars () # DEBUG
			for (d in names (histPositions)[-1]) {
				m <- min (histPositions [[d]] - len.n + 1, len.r)
				histPositions [[d]] <<- replacePositions [[d]] <<- m
				message ("d: ", d, ", m: ", m)  # DEBUG
				replay (m, d)
			}
			#printPars () # DEBUG
			.rk.graph.history.gui ()
		}
		
		if (is.null (pos)) {
			# pos == NULL means call originated from a managed device by clicking on 'Remove from history' icon,
			# it does not mean that the position on the concerned device is NULL! The actual position is
			# appropriately set below.
			
			if (is.null (deviceId)) stop ('Both deviceId and pos are NULL') # why should this happen ??
			deviceId <- as.character (deviceId)
			if (! (deviceId %in% names(histPositions))) stop (paste ('Device', deviceId, 'is not managed'))
			
			pos <- histPositions [[deviceId]] # here length (pos) = 1
			pop.and.update (n = pos)
		} else if (all(pos > 0) && all (pos <= history_length)) {
			# call from: .grow.history () and .verify.hist.limits (); not from any device
			
			pop.and.update (n = pos)
		} else
			stop (paste ('Invalid position(s)'))
		
		invisible (NULL)
	}
	replay <- function(n = histPositions [[as.character (deviceId)]] - 1L, deviceId = dev.cur ())
	{
		# when this function is called, there are NO unsaved plots! Saving the unsaved plot is taken care off
		# by the wrapper functions, showXxxxx (), below
		
		deviceId <- as.character (deviceId)
		
		if (n > 0 && n <= length(recorded)) {
			cur.deviceId <- dev.cur ()
			dev.set (as.numeric(deviceId))
		
			if (gType [[n]] == "standard") {
				replayPlot (recorded[[n]])
			} else if (gType [[n]] == "lattice") {
				# (re-)plot the lattice object but, if the current window is NOT active, then do not save
				# it to lattice:::.LatticeEnv$last.object ("trellis.last.object")
				plot (recorded[[n]], save.object = (cur.deviceId == as.numeric (deviceId)))
			}
			replacePositions [[deviceId]] <<- histPositions [[deviceId]] <<- n
			histPositions [[deviceId]] <<- n
			dev.set (cur.deviceId)
			.rk.graph.history.gui ()
		}
		else message("replay: 'n' not in valid range: ", n)
	}
	replaceby <- function (deviceId = dev.cur ())
	{
		deviceId <- as.character (deviceId)
		p <- replacePositions [[deviceId]]
		record (deviceId)
		n <- histPositions [[deviceId]]
		recorded [[p]] <<- recorded [[n]]
		gType [[p]] <<- gType [[n]]
		remove (pos = n)
		histPositions [[deviceId]] <<- p
		replay (n = p, deviceId)
		invisible ()
	}
	showFirst <- function(deviceId = dev.cur())
	{
		record (deviceId)
		replay(n = 1, deviceId)
	}
	showPrevious <- function(deviceId)
	{
		record (deviceId)
		replay(n = histPositions [[as.character (deviceId)]] - 1L, deviceId = deviceId)
	}
	showNext <- function(deviceId)
	{
		record (deviceId)
		replay(n = histPositions [[as.character (deviceId)]] + 1L, deviceId = deviceId)
	}
	showLast <- function(deviceId = dev.cur())
	{
		record (deviceId)
		replay(n = length(recorded), deviceId)
	}
	showPlot <- function(deviceId = dev.cur(), index)
	{
		# TODO: record might remove a plot form history, thus changing the indices!
		record (deviceId)
		index = max (as.integer (index), 1L)
		replay(n = min (length (recorded), index))
	}
	clearHistory <- function ()
	{
		isDuplicate <<- FALSE
		isPreviewDevice <<- FALSE
		recorded <<- list()
		gType <<- list ()
		histPositions [names (histPositions)] <<- 0
		replacePositions [names (replacePositions)] <<- 0
		#printPars () # DEBUG
		.rk.graph.history.gui ()
	}
	printPars <- function ()
	{
		message ('History length   : ', length (recorded))
		message ("History size (KB): ", round (object.size (recorded) / 1024, 2))
		message ('Current devices  : ', paste (names (histPositions), collapse = ', ')) 
		message ('Current positions: ', paste (unlist (histPositions), collapse = ', ')) 
		message ('Previos positions: ', paste (unlist (replacePositions), collapse = ', ')) 
		message ('gType @ these pos: ', paste (unlist (gType [unlist (histPositions)]), collapse = ', '))
		message ("Plot proerties   :")
		for (d in names (histPositions)[-1]) message (try (.get.plot.info.str (d)))
	}
	.rk.graph.history.gui <- function (deviceIds = names (histPositions))
	{
		# this function is called whenever the history length changes
		# or the position changes in any device.
		
		deviceIds <- deviceIds [deviceIds != "1"] # ignore NULL device
		ndevs <- length (deviceIds)
		if (ndevs>0) {
			positions <- character (1 + 2 * ndevs)
			positions [2 * (1:ndevs) - 1] <- deviceIds
			positions [2 * (1:ndevs)] <- unlist (histPositions[deviceIds], use.names = FALSE)
			labels <- NULL
			if (length (recorded) > 0) labels <- sapply (1:length (recorded), .get.oldplot.call)
			.rk.do.call ("updateDeviceHistory", c (length (recorded), labels, positions));
		}
		#print (positions) # DEBUG
		invisible (NULL)
	}
	.get.oldplot.call.std <- function (n)
	{
		# rp <- recordPlot () is a nested pairlist object (of class "recordedplot"):
		# rp[[1]] is the "meta data", rp[[2]] is always raw,
		# We then figure out the relevant stuff from rp[[1]]. Use "str (rp)" for details.
		# Currently, only main, xlab, and ylab meta data can be extracted, unambiguously.
		# The high level calls are not part of the meta data, only the low level .Internal
		#   calls are stored: Eg: .Primitive (plot.xy), .Primitive (rect), .Primitive (persp), etc...
		
		# .f. identifies which element(s) in rp[[1]] contains title (=main,sub,xlab,ylab) information:
		# differs from call to call. Eg: for plot () calls this is 7, for hist () this is 3, ...
		.f. <- function ()
			which (lapply (recorded [[n]][[1]], FUN = function (x) x[[1]]) == ".Primitive(\"title\")")
		# Sometimes there is no title information at all - happens when the high level calling function
		#   does not specifically provide any of main/sub/xlab/ylab arguemnts: Eg: persp (...)
		# Sometimes there are more than one .Primitive ("title") calls, eg, when title (...) is called
		#   explicitely after a plotting call
		
		.x. <- list (main = "", sub = "", xlab = "", ylab = "")
		
		# When present, rp [[1]] [.n.] [[2]] contains (in multiple lists) main, sub, xlab, ylab, etc.
		# From there we pick up the last (which.max) non-null entry for each of main, sub, xlab, and ylab
		.n. <- .f. ()
		if (length (.n.) > 0) {
			.T. <- lapply (lapply (recorded [[n]][[1]] [.n.], FUN = function (.a.) .a.[[2]]), 
				FUN = function (.aa.) {names (.aa.) <- c("main", "sub", "xlab", "ylab"); .aa.})
			
			for (i in c("main", "sub", "xlab", "ylab"))
				.x.[[i]] <- .T. [[ which.max (sapply (.T., FUN = function (.a.) !is.null (.a.[[i]]))) ]] [[i]]
		}
		
		paste ("Main: '", .x.$main, "'; X label: '", .x.$xlab, "'; Y label: '", .x.$ylab, "'", sep = "")
	}
	.get.oldplot.call.lattice <- function (n)
	{
		# trellis objects contain a call object which is the best meta data possible!
		# If needed, main/xlab/ylab can be extracted as well.
		paste ("Call: ", paste (deparse (recorded [[n]]$call), collapse = "\n"), sep = "")
	}
	.get.oldplot.call <- function (n)
	{
		# this can be easily extended to more types
		switch (gType [[n]],
			standard = .get.oldplot.call.std (n),
			lattice = .get.oldplot.call.lattice (n),
			"Unknown")
	}
	.get.plot.info.str <- function (deviceId = dev.cur ())
	{
		deviceId <- as.character (deviceId)
		if (!deviceId %in% names (histPositions)) return ("Preview devices is not managed.")
		
		n <- histPositions [[deviceId]]
		recorded.n.exists <- length (recorded) >= n
		if (n == 0) {
			info.str <- paste ("Device: ", deviceId, ", Position: 0", sep = "")
		} else if (!recorded.n.exists) {
			info.str <- paste ("Device: ", deviceId, ", Position: ", n, ", Size: ?\nType: ", gType [[n]], sep = "")
		} else {
			info.str <- paste ("Device: ", deviceId, 
				", Position: ", n, 
				", Size (KB): ", round (object.size (recorded [[n]])/1024, 2),
				"\n", .get.oldplot.call (n), sep = "")
		}
		info.str
	}
	showPlotInfo <- function (deviceId = dev.cur ())
	{
		rk.show.message (.get.plot.info.str (deviceId), caption = "Plot properties")
	}
	.verify.hist.limits <- function ()
	{
		# Length restriction:
		len.max <- getOption ('rk.graphics.hist.max.length')
		len.r <- length (recorded)
		
		ans <- 'no'
		if (len.max < len.r) {
			ans <- rk.show.question (paste ("Current plot history has more plots than the maximum number specified in the settings.\n",
				len.r - len.max, " of the foremost plots will be removed.\n\nDo you want to Continue?", sep =""))
			if (!is.null(ans) && ans)
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
"rk.goto.plot" <- function (deviceId = dev.cur (), index=1) {
	rk.record.plot$showPlot (deviceId, index)
	rk.record.plot$printPars ()
}
"rk.replaceby.plot" <- function (deviceId = dev.cur ())
{
	rk.record.plot$replaceby (deviceId)
	rk.record.plot$printPars ()
}
"rk.removethis.plot" <- function (deviceId = dev.cur ())
{
	rk.record.plot$remove (deviceId)
	rk.record.plot$printPars ()
}
