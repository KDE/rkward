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

"rk.duplicate.device" <- function (devId = dev.cur ())
{
	rk.record.plot$.my.message ("------- call begin -----------")
	rk.record.plot$.my.message ("in: rk.duplicate.device")
	dev.set (devId)
	dev.copy (device = x11, is.being.duplicated = TRUE)
	rk.record.plot$.my.message ("------- call end   -----------")
}

## TODO: need a wrapper around dev.set () and dev.copy!!
## o/w a user call of dev.set () and dev.copy () will not be set/initiate the history properly
"rk.activate.device" <- function (devId = dev.cur ())
{
	rk.record.plot$.my.message ("------- call begin -----------")
	dev.set (devId)
	rk.record.plot$.set.trellis.last.object ()
	rk.record.plot$getDevSummary ()
	rk.record.plot$.my.message ("------- call end   -----------")
}

# A global history of various graphics calls;
"rk.record.plot" <- function ()
{
	env <- environment()
	
	.sP.index <- list ()
	sP.length <- length (.sP.index)
	.set.sP.length <- function () sP.length <<- length (.sP.index)
	
	.sP.template <- list (plot = NULL, pkg = "", time = NULL, call = NULL)
	savedPlots <- list () # length (savedPlots) should always be == length (.sP.index) == sP.length
	.unsavedPlot <- list (plot = NULL, pkg = NA_character_, is.os = NA, tryerr = NA)
	
	.hP.template <- list (is.this.plot.new = FALSE, is.this.dev.new = TRUE, 
		pos.cur = NA_integer_, pos.prev = NA_integer_, pos.dupfrom = NA_integer_, 
		#do.record = FALSE, do.append = FALSE, do.replace = FALSE, 
		pkg = "", call = NA_character_, plot = NA)
	histPositions <- list ("1" = .hP.template)
	.hP.names <- names (histPositions)
	.set.hP.names <- function () .hP.names <<- names (histPositions)
	
	.rk.rp.debug <- FALSE
	.set.rk.rp.debug <- function (x) .rk.rp.debug <<- x
	
	.ss.used <- FALSE # split.screen variable
	.pop.notify <- TRUE # used when hist limit is reached
	.cll <- 50
	.set.call.lab.len <- function (x) .cll <<- x
	
	## Generic functions:
	.get.sys.time <- function () format (Sys.time (), "%Y%m%d%H%M%OS3")
	.is.device.managed <- function (devId) as.character (devId) %in% .hP.names[-1]
	.set.trellis.last.object <- function (devId = dev.cur ())
	{
		# called only from rk.activate.device ()
		devId <- as.character (devId)
		if (histPositions[[devId]]$pkg != "lattice") return (invisible ())
		
		.my.message ("call from .set.tlo: Will set tlo")
		if (histPositions[[devId]]$is.this.plot.new)
			tlo <- histPositions[[devId]]$plot
		else
			tlo <- savedPlots [[.sP.index [[histPositions[[devId]]$pos.cur]]]]$plot
		
		assign ("last.object", tlo, envir = lattice:::.LatticeEnv)
	}
	.is.par.or.screen.inuse <- function ()
	{
		# takes care of par (mfrow / mfcol) and split.screen () issues "almost!"
		ret <- FALSE
		if (sum (par ("mfg") * c(-1,-1,1,1)) != 0)
			ret <- TRUE
		else if (graphics:::.SSexists ("sp.screens")) {
			if (!.ss.used)
				.ss.used <<- TRUE
			else
				ret <- TRUE
		} else 
			.ss.used <<- FALSE
		ret
	}
	
	## Device specific functions:
	onAddDevice <- function (devId.from = 1, devId = dev.cur (), 
		is.being.duplicated = FALSE, is.preview.device = FALSE)
	{
		.my.message ("------- call begin -----------")
		.my.message ("in: onAddDevice")
		if (is.preview.device) return (invisible ())
		
		devId.from <- as.character (devId.from)
		devId <- as.character (devId)
		
		histPositions [[devId]] <<- .hP.template
		if (is.being.duplicated && !histPositions [[devId.from]]$is.this.dev.new) {
			.my.message ("Being duplicated")
			# devId.from > 1
			## TODO: see if so many "[[" calls can be reduced?
			histPositions [[devId]]$is.this.plot.new <<- TRUE
			histPositions [[devId]]$is.this.dev.new <<- FALSE
			histPositions [[devId]]$pkg <<- histPositions [[devId.from]]$pkg
			histPositions [[devId]]$call <<- histPositions [[devId.from]]$call
			if (!histPositions [[devId.from]]$is.this.plot.new)
				histPositions [[devId]]$pos.dupfrom <<- histPositions [[devId.from]]$pos.cur
			histPositions [[devId]]$plot <<- histPositions [[devId.from]]$plot
		}
		.set.hP.names ()
		getDevSummary ()
		.rk.update.hist.actions ()
		.my.message ("------- call end   -----------")
		invisible ()
	}
	onDelDevice <- function (devId = dev.cur())
	{
		.my.message ("------- call begin -----------")
		.my.message ("in: onDelDevice")
		devId <- as.character (devId)
		if (!(devId %in% .hP.names[-1])) return (invisible ())
		
		## TODO: ask for confirmation before appending a plot
		record (devId, action = "dev.off")
		histPositions [[devId]] <<- NULL
		.set.hP.names ()
		
		getDevSummary ()
		.my.message ("------- call end   -----------")
		invisible ()
	}
	.save.tlo.in.hP <- function (devId = dev.cur ())
	{
		.my.message ("in: .save.tlo.in.hP")
		# tlo = trellis.last.object
		## TODO: explain why this is needed
		devId <- as.character (devId)
		histPositions [[devId]]$plot <<- trellis.last.object ()
		.my.hP.print (devId)
		invisible ()
	}
	.prep.new.device <- function (devId, pkg, .cstr)
	{
		.my.message ("in: .prep.new.device")
		histPositions [[devId]]$is.this.dev.new <<- FALSE
		histPositions [[devId]]$is.this.plot.new <<- TRUE
		histPositions [[devId]]$pkg <<- pkg
		histPositions [[devId]]$call <<- .cstr
		getDevSummary ()
		invisible ()
	}
	
	## Recording functions
	record <- function(devId = dev.cur (), isManaged = NULL, action = "", callUHA = TRUE, nextplot.pkg = "", nextplot.call = NA_character_)
	{
		.my.message ("in: record")
		devId <- as.character (devId)
		
		if (is.null (isManaged)) isManaged <- .is.device.managed (devId)
		if (!isManaged) return (invisible ())
		
		if (histPositions[[devId]]$is.this.dev.new) {
			# a new device: ie after an x11 () call
			if (action == "")  
				return (invisible (.prep.new.device (devId, nextplot.pkg, nextplot.call))) # call from plot.new () / persp () / print.trellis ()
			else if (action == "force.append")
				return (invisible (rk.show.message ("Nothing to record!", "Record Warning", FALSE))) # call from rk.force.append.plot
			else
				return (invisible ()) # if needed, handle individual actions separately
		}
		
		#if (histPositions[[devId]]$pkg == "") histPositions[[devId]]$pkg <<- "graphics"
		
		newplot.in.Q <- nextplot.pkg != ""
		if (action == "force.append") {
			histPositions[[devId]]$is.this.plot.new <<- TRUE
			histPositions[[devId]]$pkg <<- "unknown"
			histPositions[[devId]]$call <<- NA_character_
		} else if (nextplot.pkg == "graphics") {
			# unless force.append is used,
			# check for par (mfrow / mfcol / mfg) and split.screen scenarios:
			if (.is.par.or.screen.inuse () && action != "dev.off") return (invisible ())
		}
		st <- .get.sys.time ()
		n <- switch (histPositions[[devId]]$pkg,
			graphics = .record.graphics (devId, action, newplot.in.Q, st),
			unknown = .record.graphics (devId, action, newplot.in.Q, st),
			lattice = .record.lattice (devId, action, newplot.in.Q, st),
			NA_integer_)
		
		.my.message ("'n' = ", n, " (RET from record.xxx)")
		.my.message ("New plot in Q? ", newplot.in.Q)
		
		if (newplot.in.Q) {
			.tmp.hP <- modifyList (.hP.template, 
				list (is.this.plot.new = TRUE, is.this.dev.new = FALSE, pkg = nextplot.pkg, call = nextplot.call))
			## TODO: check this:
			.tmp.hP$pos.prev <- ifelse (is.null (.unsavedPlot$plot) && .unsavedPlot$is.os, 
				histPositions [[devId]]$pos.prev, n)
			histPositions [[devId]] <<- .tmp.hP
		} else {
			histPositions [[devId]]$is.this.plot.new <<- FALSE
			## TODO: pos.prev ??
			if (!is.na (n)) histPositions [[devId]]$pos.cur <<- n
			if (action == "force.append") histPositions [[devId]]$plot <<- NA
		}
		
		if (callUHA) .rk.update.hist.actions ()
		getDevSummary ()
		invisible ()
	}
	.record.graphics <- function (devId, action, newplot.in.Q, st)
	{
		.my.message ("in: .record.graphics")
		.record.main (devId, "graphics")
		if (is.null (.unsavedPlot$plot)) return (invisible (NA_integer_))
		
		if (histPositions [[devId]]$is.this.plot.new) {
			save.mode <- ifelse (newplot.in.Q, "append", action)
			if (save.mode %in% c("arrows", "dev.off", "force.append")) save.mode <- "append"
		} else {
			save.mode <- ifelse (newplot.in.Q, "overwrite", action)
			if (save.mode %in% c("arrows", "dev.off")) save.mode <- "overwrite"
		}
		
		.my.message ("save.mode: ", save.mode)
		
		n <- save.plot.to.history (devId, save.mode, 
			ifelse (action == "force.append", "unknown", "graphics"), 
			st, histPositions[[devId]]$call)
		.my.message ("'n' = ", n, " (RET from save.plot.to.history)")
		invisible (n)
	}
	.record.lattice <- function (devId, action, newplot.in.Q, st)
	{
		.my.message ("in: .record.lattice")
		if (!histPositions [[devId]]$is.this.plot.new) return (invisible (histPositions [[devId]]$pos.cur))
		
		.record.main (devId, "lattice")
		if (is.null (.unsavedPlot$plot)) return (invisible (NA_integer_))
		
		save.mode <- ifelse (newplot.in.Q, "append", action)
		if (save.mode %in% c("arrows", "dev.off")) save.mode <- "append"
		
		.my.message ("save.mode: ", save.mode)#, ", check.odsp: ", check.odsp)
		
		n <- save.plot.to.history (devId, save.mode, "lattice", st, histPositions[[devId]]$call)
		.my.message ("'n' = ", n, " (RET from save.plot.to.history)")
		invisible (n)
	}
	.record.main <- function (devId, pkg)
	{
		.my.message ("in: .record.main")
		devId.cur <- dev.cur ()
		unsplot <- NULL
		if (pkg %in% c("graphics", "unknown")) {
			dev.set (as.numeric (devId))
			try (unsplot <- recordPlot(), silent=TRUE)
			dev.set (devId.cur)
		} else if  (pkg == "lattice") {
			dev.set (as.numeric (devId))
			try (unsplot <- trellis.last.object (), silent=TRUE)
			dev.set (devId.cur)
		} else {
			##TODO: is is still possible to save it?
			.unsavedPlot <<- list (plot = NULL, pkg = NA_character_, is.os = NA, tryerr = NA)
			return (invisible (rk.show.message ("Unknown graphics function. Use append to store.", "Recording error", FALSE)))
		}
		
		if (class (unsplot) == "try-error") {
			.unsavedPlot <<- list (plot = NULL, pkg = pkg, is.os = NA, tryerr = TRUE)
			return (invisible (rk.show.message ("Unknown recording error", "Recording error", FALSE)))
		}
		
		.unsavedPlot <<- list (plot = unsplot, pkg = pkg, 
			is.os = object.size (unsplot) > getOption ("rk.graphics.hist.max.plotsize") * 1024, tryerr = FALSE)
		
		invisible ()
	}
	
	## Saving (the recorded plot) functions:
	save.plot.to.history <- function (devId, save.mode, pkg, st, .cstr = NA_character_)
	{
		.my.message ("in: save.plot.to.history")
		switch (save.mode,
			append = .save.plot.to.history.append (devId, pkg, st, .cstr),
			overwrite = .save.plot.to.history.overwrite (devId, pkg, st, .cstr),
			NA_integer_)
	}
	.save.plot.to.history.append <- function (devId, pkg, st, .cstr)
	{
		.my.message ("in: .save.plot.to.history.append")
		if (!.save.oversized.plot ()) return (invisible (NA_integer_))
		
		n <- .grow.history (st)
		if (is.na (n)) return (invisible (n))
		
		savedPlots [[st]] <<- list (plot = .unsavedPlot$plot, pkg = pkg, time = st, call = NULL)
		savedPlots [[st]]$call <<- try (.get.oldplot.call (n, .cll, .cstr))
		.my.message ("'n' = ", n, " (.save.plot.to.history.append)")
		invisible (n)
	}
	.save.plot.to.history.overwrite <- function (devId, pkg, st, .cstr)
	{
		.my.message ("in: .save.plot.to.history.overwrite")
		# this is not setup to handle an (yet unwritten) 'overwrite' action from tool/menu bar
		n <- histPositions [[devId]]$pos.cur
		.st. <- .sP.index [[n]]
		if (!.check.identical (.st., pkg) && !is.null (.unsavedPlot$plot)) {
			if (!.save.oversized.plot ()) return (invisible (n))
			.my.message ("_NOT_ identical, so overwriting!")
			savedPlots [[.st.]]$plot <<- .unsavedPlot$plot
			savedPlots [[.st.]]$call <<- try (.get.oldplot.call (n, .cll, .cstr))
			.check.other.dev.at.same.pos (devId, n)
		} else .my.message ("_IS_ identical, so not ckecking for odsp")
		.my.message ("'n' = ", n, " (.save.plot.to.history.overwrite)")
		invisible (n)
	}
	.save.oversized.plot <- function ()
	{
		if (is.na (.unsavedPlot$is.os))
			ret <- FALSE
		else if (!.unsavedPlot$is.os)
			ret <- TRUE
		else 
			ret <- rk.show.question ("Large plot!\nDo you still want to store it in the history?", 
				"WARNING!", button.cancel = "")
		ret
	}
	.check.identical <- function (.st., pkg=NA_character_) {
		# this may need to be split into separate .check.identical."pkg" functions
		identical (savedPlots[[.st.]]$plot, .unsavedPlot$plot)
	}
	.check.other.dev.at.same.pos <- function (devId, .n.)
	{
		# length (.n.) >= 1 when .verify.hist.limits () calls remove ()
		.my.message ("in: .check.other.dev.at.same.pos")
		odnames <- .hP.names [!(.hP.names %in% c("1", devId))]
		.my.message ("odnames: ", paste (odnames, collapse = ", "))
		if (length (odnames) == 0) return (invisible ())
		
		odpos <- sapply (histPositions [odnames], "[[", "pos.cur") # names kept
		odpos <- odpos [which (odpos %in% .n.)]
		.my.message ("names (odpos): ", paste (names (odpos), collapse = ", "))
		.my.message ("        odpos: ", paste (odpos, collapse = ", "))
		if (length (odpos) == 0) return (invisible ())
		
		for (d in names (odpos)) {
			histPositions[[d]]$is.this.plot.new <<- TRUE
			histPositions[[d]]$pos.prev <<- histPositions[[d]]$pos.cur ## may not be approprite for "remove"
			histPositions[[d]]$pos.cur <<- NA_integer_
		}
		invisible ()
	}
	.grow.history <- function (st)
	{
		.my.message ("in: grow.history")
		len.sP <- sP.length
		ml <- getOption ('rk.graphics.hist.max.length')
		
		if (len.sP < ml) {
			n <- len.sP + 1
		} else if (len.sP == ml) {
			if (.pop.notify)
				.pop.notify <<- rk.show.question ("History limit reached, removing the first plot. Limits can be changed at Settings > RKWard > Output.\n\nDo you want to be notified in future?", 
					"WARNING!", button.cancel = "")
			remove (devId = NULL, pos = 1) # sP.length changes at this point
			n <- len.sP
		} else {
			# this can happen, if max history length gets set below sP.length, without removing the excess plots,
			# still, this should be avoided.
			rk.show.message ("Current history length > max length: plot not added to history!", "WARNING!")
			return (invisible (NA_integer_))
		}
		.sP.index [[n]] <<- st
		.set.sP.length ()
		.my.message ("'n' = ", n, " (grow.history)")
		n
	}
	
	## Removal function:
	remove <- function (devId = dev.cur (), pos = NA_integer_) # pos can be of length > 1
	{
		.my.message ("in: remove")
		# devId == NULL when called from verify.hist.length ()
		
		if (sP.length == 1) {
			clearHistory ()
			rk.show.message ("Plot history cleared!", "Remove plot", FALSE)
		}
		if (sP.length <= 1) return (invisible ())
		
		if (!is.null (devId)) devId <- as.character (devId)
		
		if (!is.null (devId)) {
			if (histPositions[[devId]]$is.this.dev.new) # on an empty device
				return (invisible (rk.show.message ("Nothing to remove!", "Remove plot", FALSE)))
			else if (is.na (pos) || histPositions[[devId]]$is.this.plot.new) {
				.my.message ("removing unsaved plot from device", devId, " @ pos", pos)
				# unsaved plot on the device, so just replay the "previous" plot
				.p. <- histPositions[[devId]]$pos.prev
				if (is.na (.p.)) .p. <- sP.length
				replay (.p., devId)
				return (invisible ())
			}
		}
		
		.my.message ("pos: ", paste (pos, collapse = ","))
		.check.other.dev.at.same.pos (devId, pos) # works for devId = NULL as well
		
		.my.message ("sP.length: ", sP.length)
		.sP.pos <- unlist (.sP.index [pos])
		savedPlots [.sP.pos] <<- NULL
		.sP.index [pos] <<- NULL
		.set.sP.length ()
		.my.message ("sP.length: ", sP.length)
		
		if (!is.null (devId)) replay (min (pos, sP.length), devId) # in this case, length (pos) == 1
		
		.l. <- length (pos)
		hP.gt.pos <- sapply (histPositions, "[[", "pos.cur")
		hP.gt.pos <- hP.gt.pos [which (hP.gt.pos > pos[.l.])] # removes NAs; pos[.l.] == max (pos)
		.my.message ("names (hP.gt.pos): ", names (hP.gt.pos))
		.my.message ("       hP.gt.pos : ", hP.gt.pos)
		if (length (hP.gt.pos) > 0) {
			for (.d. in names (hP.gt.pos)) {
				histPositions[[.d.]]$pos.cur <<- min (histPositions [[.d.]]$pos.cur - .l., sP.length)
				histPositions[[.d.]]$pos.prev <<- min (histPositions [[.d.]]$pos.prev - .l., sP.length)
			}
		}
		
		.rk.update.hist.actions ()
		invisible ()
	}
	clearHistory <- function ()
	{
		.my.message ("------- call begin -----------")
		.my.message ("in: clearHistory")
		.sP.index <<- list (); .set.sP.length ()
		savedPlots <<- list ()
		.unsavedPlot <<- list (plot = NULL, pkg = NA_character_, is.os = NA, tryerr = NA)
		.ss.used <<- FALSE
		for (d in .hP.names[-1]) {
			if (!histPositions [[d]]$is.this.dev.new)
				histPositions [[d]]$is.this.plot.new <<- TRUE
			histPositions [[d]]$pos.cur <<- NA_integer_
			histPositions [[d]]$pos.prev <<- NA_integer_
			histPositions [[d]]$pos.dupfrom <<- NA_integer_
			# do not reset "pkg" and "call" elements
		}
		.rk.update.hist.actions ()
		getDevSummary ()
		.my.message ("------- call end   -----------")
		invisible ()
	}
	
	## Replay function:
	replay <- function(n, devId = dev.cur ())
	{
		.my.message ("in: replay")
		.my.message ("'n' = ", n, " (replay)")
		on.exit (.rk.update.hist.actions ())
		if (missing (n))
			return (invisible (rk.show.message ("Position missing", "Replay error", FALSE)))
		if (is.na (n) || n < 0 || n > sP.length)
			return (invisible (rk.show.message(paste ("replay: 'n' not in valid range: ", n), "Replay error", FALSE)))
		
		devId <- as.character (devId)
		cur.devId <- dev.cur ()
		dev.set (as.numeric(devId))
		
		st <- .sP.index [[n]]
		pkg <- savedPlots [[st]]$pkg
		
		if (pkg %in% c("graphics", "unknown")) {
			replayPlot (savedPlots [[st]]$plot)
		} else if (pkg == "lattice") {
			# (re-)plot the lattice object but, if the current window is NOT active, then do not save
			# it to lattice:::.LatticeEnv$last.object ("trellis.last.object")
			plot (savedPlots [[st]]$plot, save.object = (cur.devId == as.numeric (devId)))
		}
		histPositions [[devId]] <<- modifyList (.hP.template, 
			list (is.this.plot.new = FALSE, is.this.dev.new = FALSE, pos.prev = n, pos.cur = n, pkg = pkg, call = savedPlots [[st]]$call))
		.my.hP.print (devId)
		dev.set (cur.devId)
		invisible()
	}
	
	## Action wrappers:
	showFirst <- function(devId = dev.cur())
	{
		if (!.is.device.managed (devId)) return (invisible ())
		record (devId, isManaged = TRUE, action = "arrows")
		replay(n = 1, devId)
	}
	showPrevious <- function(devId)
	{
		if (!.is.device.managed (devId)) return (invisible ())
		record (devId, isManaged = TRUE, action = "arrows")
		.n. <- histPositions [[as.character (devId)]]$pos.cur - 1L
		if (is.na (.n.)) .n. <- sP.length # this happens when sP.length > 0 and the user calls x11 ()
		replay(n = .n., devId = devId)
	}
	showNext <- function(devId)
	{
		if (!.is.device.managed (devId)) return (invisible ())
		record (devId, isManaged = TRUE, action = "arrows")
		replay(n = histPositions [[as.character (devId)]]$pos.cur + 1L, devId = devId)
	}
	showLast <- function(devId = dev.cur())
	{
		if (!.is.device.managed (devId)) return (invisible ())
		record (devId, isManaged = TRUE, action = "arrows")
		replay(n = sP.length, devId)
	}
	showPlot <- function(devId = dev.cur(), index)
	{
		.my.message ("in: showPlot")
		if (!.is.device.managed (devId)) return (invisible ())
		
		.n. <- histPositions [[as.character (devId)]]$pos.cur
		if (index == ifelse (is.na (.n.), sP.length + 1, .n.)) {
			.my.message ("Same position! No action needed.")
			return (invisible ())
		}
		
		## TODO: record might remove a plot form history, thus changing the indices!
		record (devId, isManaged = TRUE, action = "arrows")
		.my.message ("index: ", index)
		index <- max (as.integer (index), 1L)
		.my.message ("index: ", index, " (after max)")
		.my.message ("'n': ", min (sP.length, index), " (still in showPlot)")
		replay(n = min (sP.length, index), devId)
	}
	forceAppend <- function (devId = dev.cur ())
	{
		if (!.is.device.managed (devId)) return (invisible (rk.show.message ("Device not managed", "Append this plot", FALSE)))
		record (devId, isManaged = TRUE, action = "force.append")
	}
	removePlot <- function (devId = dev.cur ())
	{
		if (!.is.device.managed (devId)) return (invisible (rk.show.message ("Device not managed", "Remove plot", FALSE)))
		remove (devId, histPositions[[as.character (devId)]]$pos.cur)
	}
	showPlotInfo <- function (devId = dev.cur ())
	{
		.my.message ("in: showPlotInfo")
		rk.show.message (.get.plot.info.str (devId), caption = "Plot properties")
	}
	
	## Utility / print functions:
	getDevSummary <- function ()
	{
		if (!.rk.rp.debug) return (invisible ())
		message ('History length   : ', sP.length)
		message ("History size (KB): ", round (object.size (savedPlots) / 1024, 2))
		.my.hP.print ()
	}
	getSavedPlotsSummary <- function ()
	{
		.my.message ("------- call begin -----------")
		.tmp.df <- data.frame (
			call = sapply (savedPlots[unlist (.sP.index, use.names = FALSE)], "[[", "call"),
			size.KB  = sapply (lapply (savedPlots[unlist (.sP.index, use.names = FALSE)], "[[", "plot"), function (x) object.size(x)/1024),
			pkg  = sapply (savedPlots[unlist (.sP.index, use.names = FALSE)], "[[", "pkg"),
			timestamp  = sapply (savedPlots[unlist (.sP.index, use.names = FALSE)], "[[", "time"))
		rownames (.tmp.df) <- NULL
		.my.message ("------- call end   -----------")
		.tmp.df
	}
	.my.message <- function (...) if (.rk.rp.debug) message (paste (..., sep = " "))
	.my.hP.print <- function (devId = NULL) {
		if (!.rk.rp.debug) return (invisible ())
		if (is.null (devId)) {
			.tmp.df <- data.frame (
				pNew = sapply (histPositions, "[[", "is.this.plot.new"),
				dNew = sapply (histPositions, "[[", "is.this.dev.new"),
				posC = sapply (histPositions, "[[", "pos.cur"),
				posP = sapply (histPositions, "[[", "pos.prev"),
				posD = sapply (histPositions, "[[", "pos.dupfrom"),
				pkg  = sapply (histPositions, "[[", "pkg"),
				pCls  = sapply (lapply (histPositions, "[[", "plot"), FUN = function (x) class (x)))
			rownames (.tmp.df) <- names (histPositions)
		} else {
			devId <- as.character (devId)
			.a.hP <- histPositions[[devId]]
			.tmp.df <- data.frame (
				pNew = .a.hP$is.this.plot.new,
				dNew = .a.hP$is.this.dev.new,
				posC = .a.hP$pos.cur,
				posP = .a.hP$pos.prev,
				posD = .a.hP$pos.dupfrom,
				pkg  = .a.hP$pkg,
				pCls  = class (.a.hP$plot))
			rownames (.tmp.df) <- devId
		}
		sink (file = stderr (), type = "output")
		print (.tmp.df)
		sink (file = stdout (), type = "output")
	}
	
	## Utility / call labels functions:
	.get.sP.calls <- function ()
	{
		labels <- NULL
		if (sP.length > 0)
			labels <- paste (format (1:sP.length), sapply (savedPlots [unlist (.sP.index, use.names = FALSE)], "[[", "call"), sep = ": ")
		names (labels) <- NULL
		labels
	}
	.get.plot.info.str <- function (devId = dev.cur (), l=0)
	{
		devId <- as.character (devId)
		if (!(devId %in% .hP.names)) return (invisible (rk.show.message (paste ("Device", devId, "is not managed."), wait = FALSE)))
		
		n <- histPositions [[devId]]$pos.cur
		if (is.na (n)) {
			info.str <- paste ("Device: ", devId, ", Position: <new>, Size: ?\nType: ", histPositions [[devId]]$pkg, sep = "")
		} else if (n == 0) {
			info.str <- paste ("Device: ", devId, ", Position: 0", sep = "")
		} else {
			info.str <- paste ("Device: ", devId, 
				", Position: ", n, 
				", Size (KB): ", round (object.size (savedPlots [[.sP.index [[n]]]]$plot)/1024, 2),
				"\n", .get.oldplot.call (n, l, histPositions [[devId]]$call), sep = "")
		}
		info.str
	}
	.get.oldplot.call <- function (n, l=0, cs = NA_character_)
	{
		# this can be easily extended to more types
		switch (savedPlots [[.sP.index [[n]]]]$pkg,
			graphics = .get.oldplot.call.std2 (l, cs),
			unknown = .get.oldplot.call.std (n, l),
			lattice = .get.oldplot.call.lattice (n, l),
			"Unknown")
	}
	.get.oldplot.call.std <- function (n,l=0)
	{
		# rp <- recordPlot () is a nested pairlist object (of class "recordedplot"):
		# rp[[1]] is the "meta data", rp[[2]] is always raw,
		# We then figure out the relevant stuff from rp[[1]]. Use "str (rp)" for details.
		# Currently, only main, sub, xlab, and ylab meta data can be extracted, unambiguously.
		# The high level calls are not part of the meta data, only the low level .Internal
		#   calls are stored: Eg: .Primitive (plot.xy), .Primitive (rect), .Primitive (persp), etc...
		
		# .f. identifies which element(s) in rp[[1]] contains title (=main,sub,xlab,ylab) information:
		# differs from call to call. Eg: for plot () calls this is 7, for hist () this is 3, ...
		.tmp.plot. <- savedPlots [[.sP.index [[n]]]]$plot[[1]]
		.f. <- function ()
			which (lapply (.tmp.plot., FUN = function (x) x[[1]]) == ".Primitive(\"title\")")
		# Sometimes there is no title information at all - happens when the high level calling function
		#   does not specifically provide any of main/sub/xlab/ylab arguemnts: Eg: persp (...)
		# Sometimes there are more than one .Primitive ("title") calls, eg, when title (...) is called
		#   explicitely after a plotting call
		
		.x. <- list (main = "", sub = "", xlab = "", ylab = "")
		
		# When present, rp [[1]] [.n.] [[2]] contains (in multiple lists) main, sub, xlab, ylab, etc.
		# From there we pick up the last (which.max) non-null entry for each of main, sub, xlab, and ylab
		.n. <- .f. ()
		if (length (.n.) > 0) {
			.T. <- lapply (lapply (.tmp.plot. [.n.], FUN = function (.a.) .a.[[2]]), 
				FUN = function (.aa.) {names (.aa.) <- c("main", "sub", "xlab", "ylab"); .aa.})
			
			for (i in c("main", "sub", "xlab", "ylab"))
				.x.[[i]] <- .T. [[ which.max (sapply (.T., FUN = function (.a.) !is.null (.a.[[i]]))) ]] [[i]]
		}
		
		#.lab.str <- paste ("Main: '", .x.$main, "'; X label: '", .x.$xlab, "'; Y label: '", .x.$ylab, "'", sep = "")
		.lab.str <- paste ("X: ", .x.$xlab, "; Y: ", .x.$ylab, "; ", .x.$main, sep = "")
		if (l <= 0 || nchar (.lab.str) <= l) return (.lab.str)
		
		paste (substr (.lab.str, 1, l), "...", sep = "")
	}
	.get.oldplot.call.std2 <- function (l=0, cs)
	{
		.lab.str <- paste (ifelse (is.call (cs), deparse (cs), cs), collapse = ifelse (l<=0, "\n", ", "))
		if (l <= 0 || nchar (.lab.str) <= l) return (.lab.str)
		
		paste (substr (.lab.str, 1, l), "...", sep = "")
	}
	.get.oldplot.call.lattice <- function (n,l=0)
	{
		.lab.str <- paste (deparse (savedPlots [[.sP.index [[n]]]]$plot$call), collapse = ifelse (l<=0, "\n", ", "))
		if (l <= 0 || nchar (.lab.str) <= l) return (.lab.str)
		
		paste (substr (.lab.str, 1, l), "...", sep = "")
	}
	
	## Utility / R - C++ connection functions:
	.rk.update.hist.actions <- function (devIds = .hP.names)
	{
		# this function is called whenever the history length changes
		# or the position changes in any device.
		
		devIds <- devIds [devIds != "1"] # ignore NULL device
		ndevs <- length (devIds)
		if (ndevs > 0) {
			positions <- character (2 * ndevs)
			positions [2 * (1:ndevs) - 1] <- devIds
			ihP <- sapply (histPositions[devIds], "[[", "pos.cur"); ihP [is.na (ihP)] <- sP.length + 1
			positions [2 * (1:ndevs)] <- ihP
			#labels <- NULL
			#if (sP.length > 0) labels <- sapply (1:sP.length, function (x) try (.get.oldplot.call (x, .cll)))
			labels <- .get.sP.calls ()
			.rk.do.call ("updateDeviceHistory", c (sP.length, labels, positions));
			.my.message ("uDHA call:")
			.my.message ("  length: ", sP.length)
			.my.message ("  positions: ", paste (positions, collapse = ", "))
			.my.message ("  labels: ", ifelse (is.null (labels), "NULL", paste ("\n   ", paste (labels, collapse = "\n    "))))
		}
		invisible ()
	}
	.verify.hist.limits <- function (len.max)
	{
		.my.message ("in: verify.hist.limits")
		# this is called from settings/rksettingsmoduleoutput.cpp ~199
		# Length restriction:
		if (len.max < sP.length) {
			ans <- rk.show.question (paste ("Current plot history has more plots than the specified limit.\nIf you continue then _",
				sP.length - len.max, "_ of the foremost plots will be removed.\nInstead, if you ignore then the new limit will be effective only after restarting RKWard.", sep =""), 
				"WARNING!",
				button.yes = "Continue", button.no = "Ignore for this session", button.cancel = "")
			if (ans) {
				options ("rk.graphics.hist.max.length" = len.max)
				remove (devId = NULL, pos = 1:(sP.length - len.max))
			}
		} else {
			# this takes care of the initialization when RKWard starts..
			options ("rk.graphics.hist.max.length" = len.max)
		}
		
		# Size restriction:
		#s <- getOption ('rk.graphics.hist.max.plotsize')
		# Existing plots are not checked for their sizes, only the new ones are.
	}

	env
}
rk.record.plot <- rk.record.plot ()

# Users should use only these wrappers:
# 1 is always the null device
# TODO : comment / remove getDevSummary call
"rk.first.plot" <- function (devId = dev.cur ())
{
	rk.record.plot$.my.message ("------- call begin -----------")
	rk.record.plot$showFirst (devId)
	rk.record.plot$getDevSummary ()
	rk.record.plot$.my.message ("------- call end   -----------")
}
"rk.previous.plot" <- function (devId = dev.cur ())
{
	rk.record.plot$.my.message ("------- call begin -----------")
	rk.record.plot$showPrevious (devId)
	rk.record.plot$getDevSummary ()
	rk.record.plot$.my.message ("------- call end   -----------")
}
"rk.next.plot" <- function (devId = dev.cur ())
{
	rk.record.plot$.my.message ("------- call begin -----------")
	rk.record.plot$showNext (devId)
	rk.record.plot$getDevSummary ()
	rk.record.plot$.my.message ("------- call end   -----------")
}
"rk.last.plot" <- function (devId = dev.cur ())
{
	rk.record.plot$.my.message ("------- call begin -----------")
	rk.record.plot$showLast (devId)
	rk.record.plot$getDevSummary ()
	rk.record.plot$.my.message ("------- call end   -----------")
}
"rk.goto.plot" <- function (devId = dev.cur (), index=1) {
	rk.record.plot$.my.message ("------- call begin -----------")
	rk.record.plot$.my.message ("in: goto.plot")
	rk.record.plot$.my.message ("index: ", index)
	rk.record.plot$showPlot (devId, index)
	rk.record.plot$getDevSummary ()
	rk.record.plot$.my.message ("------- call end   -----------")
}
"rk.force.append.plot" <- function (devId = dev.cur ())
{
	rk.record.plot$.my.message ("------- call begin -----------")
	rk.record.plot$forceAppend (devId)
	rk.record.plot$getDevSummary ()
	rk.record.plot$.my.message ("------- call end   -----------")
}
"rk.removethis.plot" <- function (devId = dev.cur ())
{
	rk.record.plot$.my.message ("------- call begin -----------")
	rk.record.plot$removePlot (devId)
	rk.record.plot$getDevSummary ()
	rk.record.plot$.my.message ("------- call end   -----------")
}
"rk.clear.plot.history" <- function ()
{
	rk.record.plot$.my.message ("------- call begin -----------")
	rk.record.plot$clearHistory ()
	rk.record.plot$.my.message ("------- call end   -----------")
}
"rk.show.plot.info" <- function (devId = dev.cur ())
{
	rk.record.plot$.my.message ("------- call begin -----------")
	rk.record.plot$showPlotInfo (devId)
	rk.record.plot$.my.message ("------- call end   -----------")
}
"rk.verify.plot.hist.limits" <- function (lmax)
{
	rk.record.plot$.my.message ("------- call begin -----------")
	rk.record.plot$.verify.hist.limits (as.integer (lmax))
	rk.record.plot$.my.message ("------- call end   -----------")
}
