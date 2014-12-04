local({
## Vorbereiten
require(pwr)
## Berechne
	pwr.result <- try(
		pwr.t.test(
			n=30,
			d=0.30
		)
	)

## Drucke Ergebnisse
	# Catch errors due to unsuitable data
	if(class(pwr.result) == "try-error"){
		rk.print("Power anaylsis not possible with the data you provided")
		return()
	}

	# Prepare printout
	note <- pwr.result[["note"]]
	parameters <- list("Target measure"="Power")
	if(!is.null(pwr.result[["alternative"]])){
		parameters[["alternative"]] <- pwr.result[["alternative"]]
	}

	rk.header(pwr.result[["method"]], parameters=parameters)
	pwr.result[c("method", "note", "alternative")] <- NULL
	pwr.result <- as.data.frame(unlist(pwr.result))
	colnames(pwr.result) <- "Parameters"

	rk.results(pwr.result)
	if(!is.null(note)){
		rk.print(paste("<strong>Note:</strong> ", note))
	}

	rk.print("Interpretation of effect size <strong>d</strong> (according to Cohen):")
	rk.results(data.frame(small=0.2, medium=0.5, large=0.8))
})
