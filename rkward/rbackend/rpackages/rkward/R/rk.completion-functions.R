#' Helper functions for querying completions
#' 
#' These are currently not exported, as they are not intended to be use in user code
#' The API may or may not be stable.
#'
#' @rdname rk.get.completions
#'
".rk.completions" <- function(fragment, type) {
#TODO: factor out all the common code
	oldrcs <- utils::rc.settings()
	oldrcopts <- utils::rc.options()
	on.exit({do.call(utils::rc.settings, as.list(oldrcs)); utils::rc.options(oldrcopts)})
	if (type == "funargs") {
		utils::rc.settings(ops=FALSE, ns=FALSE, args=TRUE, dots=FALSE, func=FALSE, ipck=FALSE, S3=TRUE, data=FALSE, help=FALSE, argdb=TRUE, fuzzy=FALSE, quotes=FALSE, files=FALSE)
		utils::rc.options(funarg.suffix="")
		utils:::.assignLinebuffer(paste0(fragment, "("))
		utils:::.assignToken("")
		utils:::.assignStart(nchar(fragment)+1)
		utils:::.assignEnd(nchar(fragment)+1)
		utils:::.completeToken()
		utils:::.retrieveCompletions()
	} else if (type == "$" || type == "@") {
		utils::rc.settings(ops = TRUE, ns=FALSE, args = FALSE, dots = FALSE, func = FALSE, ipck = FALSE, S3 = FALSE, data = FALSE, help = FALSE, argdb = FALSE, fuzzy = FALSE, quotes = FALSE, files = FALSE)
		utils:::.assignLinebuffer(paste0(fragment, type))
		utils:::.assignToken(paste0(fragment, type))
		utils:::.assignStart(1)
		utils:::.assignEnd(nchar(fragment) + nchar(type))
		utils:::.completeToken()
		utils:::.retrieveCompletions()
	} else if (type == "::" || type == ":::") {
		utils::rc.settings(ops = FALSE, ns=TRUE, args = FALSE, dots = FALSE, func = FALSE, ipck = FALSE, S3 = FALSE, data = FALSE, help = FALSE, argdb = FALSE, fuzzy = FALSE, quotes = FALSE, files = FALSE)
		utils:::.assignLinebuffer(paste0(fragment, type))
		utils:::.assignToken(paste0(fragment, type))
		utils:::.assignStart(1)
		utils:::.assignEnd(nchar(fragment) + nchar(type))
		utils:::.completeToken()
		utils:::.retrieveCompletions()
	} else if (type == "?") {
		utils::rc.settings(ops = FALSE, ns=FALSE, args = FALSE, dots = FALSE, func = FALSE, ipck = FALSE, S3 = FALSE, data = FALSE, help = TRUE, argdb = FALSE, fuzzy = FALSE, quotes = FALSE, files = FALSE)
		utils:::.assignLinebuffer(paste0(type, fragment))
		utils:::.assignToken(paste0(type, fragment))
		utils:::.assignStart(1)
		utils:::.assignEnd(nchar(fragment) + nchar(type))
		utils:::.completeToken()
		utils:::.retrieveCompletions()
	}
}
