#' @export

# this simple class is for XML and JavaScript generation,
# produced by rk.plotOptions()

#' @include rk.JS.var-class.R
#' @include rk.JS.vars.R
#' @include rk.XML.embed.R

setClass("rk.plot.opts",
	representation=representation(
		XML="XiMpLe.node",
		preprocess="rk.JS.var",
		printout="rk.JS.var",
		calculate="rk.JS.var"
	),
	prototype(
		XML=rk.XML.embed(component="rkward::plot_options", button=TRUE, label="Generic plot options", id.name="auto"),
		preprocess=rk.JS.vars(
			rk.XML.embed(component="rkward::plot_options", button=TRUE, label="Generic plot options", id.name="auto"),
			modifiers="code.preprocess", check.modifiers=FALSE),
		printout=rk.JS.vars(
			rk.XML.embed(component="rkward::plot_options", button=TRUE, label="Generic plot options", id.name="auto"),
			modifiers="code.printout", check.modifiers=FALSE),
		calculate=rk.JS.vars(
			rk.XML.embed(component="rkward::plot_options", button=TRUE, label="Generic plot options", id.name="auto"),
			modifiers="code.calculate", check.modifiers=FALSE)
	)
)

# setValidity("rk.plot.opts", function(object){
# 	return(TRUE)
# })
