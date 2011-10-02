#' @export

# this simple class is for JavaScript generation,
# produced by ite()

setClass("rk.JS.ite",
	representation=representation(
		ifJS="character",
		thenJS="character",
		elseJS="character"
	),
	prototype(
		ifJS=character(),
		thenJS=character(),
		elseJS=character()
	)
)
