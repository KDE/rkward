local({
## Prepare
require(pwr)
## Compute
	pwr.result <- try(
		pwr.t.test(
			n=30,
			d=0.30
		)
	)

## Print result
	# Catch errors due to unsuitable data
	if(class(pwr.result) == "try-error"){
		rk.print("Power analysis not possible with the data you provided")
		return()
	}

	# Prepare printout
	note <- pwr.result[["note"]]
	parameters <- list("Parameter to determine"="Power of test")
	if(!is.null(pwr.result[["alternative"]])){
		parameters[["alternative"]] <- pwr.result[["alternative"]]
	}

	rk.header(pwr.result[["method"]], parameters=parameters)
	pwr.result[c("method", "note", "alternative")] <- NULL
	pwr.result <- as.data.frame(unlist(pwr.result))
	colnames(pwr.result) <- "Parameters"

	rk.results(pwr.result)
	if(!is.null(note)){
		rk.print(paste("<strong>Note:</strong>", note))
	}

	rk.print("Interpretation of effect size <strong>d</strong> (according to Cohen):")
	rk.results(data.frame("small"=0.2, "medium"=0.5, "large"=0.8))
})
local({
## Prepare
require(pwr)
## Compute
	pwr.result <- try(
		pwr.t2n.test(
			n1=27,
			n2=33,
			d=0.30
		)
	)

## Print result
	# Catch errors due to unsuitable data
	if(class(pwr.result) == "try-error"){
		rk.print("Power analysis not possible with the data you provided")
		return()
	}

	# Prepare printout
	note <- pwr.result[["note"]]
	parameters <- list("Parameter to determine"="Power of test")
	if(!is.null(pwr.result[["alternative"]])){
		parameters[["alternative"]] <- pwr.result[["alternative"]]
	}

	rk.header(pwr.result[["method"]], parameters=parameters)
	pwr.result[c("method", "note", "alternative")] <- NULL
	pwr.result <- as.data.frame(unlist(pwr.result))
	colnames(pwr.result) <- "Parameters"

	rk.results(pwr.result)
	if(!is.null(note)){
		rk.print(paste("<strong>Note:</strong>", note))
	}

	rk.print("Interpretation of effect size <strong>d</strong> (according to Cohen):")
	rk.results(data.frame("small"=0.2, "medium"=0.5, "large"=0.8))
})
local({
## Prepare
require(pwr)
## Compute
	pwr.result <- try(
		pwr.r.test(
			r=0.30,
			power=0.81
		)
	)

## Print result
	# Catch errors due to unsuitable data
	if(class(pwr.result) == "try-error"){
		rk.print("Power analysis not possible with the data you provided")
		return()
	}

	# Prepare printout
	note <- pwr.result[["note"]]
	parameters <- list("Parameter to determine"="Sample size")
	if(!is.null(pwr.result[["alternative"]])){
		parameters[["alternative"]] <- pwr.result[["alternative"]]
	}

	rk.header(pwr.result[["method"]], parameters=parameters)
	pwr.result[c("method", "note", "alternative")] <- NULL
	pwr.result <- as.data.frame(unlist(pwr.result))
	colnames(pwr.result) <- "Parameters"

	rk.results(pwr.result)
	if(!is.null(note)){
		rk.print(paste("<strong>Note:</strong>", note))
	}

	rk.print("Interpretation of effect size <strong>r</strong> (according to Cohen):")
	rk.results(data.frame("small"=0.1, "medium"=0.3, "large"=0.5))
})
local({
## Prepare
require(pwr)
## Compute
	pwr.result <- try(
		pwr.chisq.test(
			w=0.30,
			N=30,
			df=32,
			sig.level=NULL,
			power=0.81
		)
	)

## Print result
	# Catch errors due to unsuitable data
	if(class(pwr.result) == "try-error"){
		rk.print("Power analysis not possible with the data you provided")
		return()
	}

	# Prepare printout
	note <- pwr.result[["note"]]
	parameters <- list("Parameter to determine"="Significance level")
	if(!is.null(pwr.result[["alternative"]])){
		parameters[["alternative"]] <- pwr.result[["alternative"]]
	}

	rk.header(pwr.result[["method"]], parameters=parameters)
	pwr.result[c("method", "note", "alternative")] <- NULL
	pwr.result <- as.data.frame(unlist(pwr.result))
	colnames(pwr.result) <- "Parameters"

	rk.results(pwr.result)
	if(!is.null(note)){
		rk.print(paste("<strong>Note:</strong>", note))
	}

	rk.print("Interpretation of effect size <strong>w</strong> (according to Cohen):")
	rk.results(data.frame("small"=0.1, "medium"=0.3, "large"=0.5))
})
local({
## Prepare
require(pwr)
## Compute
	pwr.result <- try(
		pwr.2p.test(
			h=0.30,
			n=30,
			sig.level=NULL,
			power=0.81,
			alternative="greater"
		)
	)

## Print result
	# Catch errors due to unsuitable data
	if(class(pwr.result) == "try-error"){
		rk.print("Power analysis not possible with the data you provided")
		return()
	}

	# Prepare printout
	note <- pwr.result[["note"]]
	parameters <- list("Parameter to determine"="Significance level")
	if(!is.null(pwr.result[["alternative"]])){
		parameters[["alternative"]] <- pwr.result[["alternative"]]
	}

	rk.header(pwr.result[["method"]], parameters=parameters)
	pwr.result[c("method", "note", "alternative")] <- NULL
	pwr.result <- as.data.frame(unlist(pwr.result))
	colnames(pwr.result) <- "Parameters"

	rk.results(pwr.result)
	if(!is.null(note)){
		rk.print(paste("<strong>Note:</strong>", note))
	}

	rk.print("Interpretation of effect size <strong>h</strong> (according to Cohen):")
	rk.results(data.frame("small"=0.2, "medium"=0.5, "large"=0.8))
})
local({
## Prepare
require(pwr)
## Compute
	pwr.result <- try(
		pwr.norm.test(
			d=0.30,
			n=30,
			sig.level=NULL,
			power=0.80
		)
	)

## Print result
	# Catch errors due to unsuitable data
	if(class(pwr.result) == "try-error"){
		rk.print("Power analysis not possible with the data you provided")
		return()
	}

	# Prepare printout
	note <- pwr.result[["note"]]
	parameters <- list("Parameter to determine"="Significance level")
	if(!is.null(pwr.result[["alternative"]])){
		parameters[["alternative"]] <- pwr.result[["alternative"]]
	}

	rk.header(pwr.result[["method"]], parameters=parameters)
	pwr.result[c("method", "note", "alternative")] <- NULL
	pwr.result <- as.data.frame(unlist(pwr.result))
	colnames(pwr.result) <- "Parameters"

	rk.results(pwr.result)
	if(!is.null(note)){
		rk.print(paste("<strong>Note:</strong>", note))
	}

	rk.print("Interpretation of effect size <strong>d</strong> (according to Cohen):")
	rk.results(data.frame("small"=0.2, "medium"=0.5, "large"=0.8))
})
local({
## Prepare
require(pwr)
## Compute
	pwr.result <- try(
		pwr.f2.test(
			v=30,
			f2=0.30,
			sig.level=0.10,
			power=0.80
		)
	)

## Print result
	# Catch errors due to unsuitable data
	if(class(pwr.result) == "try-error"){
		rk.print("Power analysis not possible with the data you provided")
		return()
	}

	# Prepare printout
	note <- pwr.result[["note"]]
	parameters <- list("Parameter to determine"="Parameter count")
	if(!is.null(pwr.result[["alternative"]])){
		parameters[["alternative"]] <- pwr.result[["alternative"]]
	}

	rk.header(pwr.result[["method"]], parameters=parameters)
	pwr.result[c("method", "note", "alternative")] <- NULL
	pwr.result <- as.data.frame(unlist(pwr.result))
	colnames(pwr.result) <- "Parameters"

	rk.results(pwr.result)
	if(!is.null(note)){
		rk.print(paste("<strong>Note:</strong>", note))
	}

	rk.print("Interpretation of effect size <strong>f<sup>2</sup></strong> (according to Cohen):")
	rk.results(data.frame("small"=0.02, "medium"=0.15, "large"=0.35))
})
