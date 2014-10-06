local ({
	x <- {e <- warpbreaks[["tension"]]; if (is.factor (e)) {levels (e)} else {sort (unique (e, nmax=10000))}}
	if (length (x) > 100) x <- c (x[1:100], "____LIMIT____")
	if (is.character (x)) { op <- options ('useFancyQuotes'=FALSE); x <- dQuote (x); options (op) }
	x
})
local({
## Compute
input <- warpbreaks[["tension"]]
# Use as.character() as intermediate data format, to support adding and dropping levels
recoded <- as.character (warpbreaks[["tension"]])
recoded[input == "L"] <- "low"
recoded[input %in% c("M","H")] <- "midorhigh"
.GlobalEnv$recoded <- as.factor (recoded)
## Print result
rk.header("Recode categorical data", parameters=list("Input variable", "warpbreaks[[\"tension\"]]",
	"Output variable", "recoded",
	"Number of differences after recoding", sum (warpbreaks[["tension"]] != recoded, na.rm=TRUE) + sum (is.na (warpbreaks[["tension"]]) != is.na (recoded))))
})
local ({
	x <- {e <- withnas; if (is.factor (e)) {levels (e)} else {sort (unique (e, nmax=10000))}}
	if (length (x) > 100) x <- c (x[1:100], "____LIMIT____")
	if (is.character (x)) { op <- options ('useFancyQuotes'=FALSE); x <- dQuote (x); options (op) }
	x
})
local({
## Compute
input <- withnas
recoded <- as.logical (rep (NA, length.out = length (withnas)))
recoded[input %in% c("2","3","4","5","6","7","8","9","10")] <- FALSE
recoded[input %in% c("9","10")] <- NA
recoded[is.na (input)] <- TRUE

warning ("Some input values were specified more than once: ", "\"9\", \"10\"")
.GlobalEnv$recoded2 <- recoded
## Print result
rk.header("Recode categorical data", parameters=list("Input variable", "withnas",
	"Output variable", "recoded2",
	"Number of differences after recoding", sum (withnas != recoded2, na.rm=TRUE) + sum (is.na (withnas) != is.na (recoded2))))
})
