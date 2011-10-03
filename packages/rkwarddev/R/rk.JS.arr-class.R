#' @export

# this simple class is for JavaScript generation,
# produced by rk.JS.array()

setClass("rk.JS.arr",
	representation=representation(
		arr.name="character",
		opt.name="character",
		IDs="vector",
		variables="vector",
		funct="character",
		option="character"
	),
	prototype(
		arr.name=character(),
		opt.name=character(),
		IDs=c(),
		variables=c(),
		funct="c",
		option=character()
	)
)
