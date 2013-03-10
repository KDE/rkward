#' Create an object for plot options in RKWard plugins
#' 
#' Generates XML and JavaScript code snippets by calling \code{rk.XML.embed} and \code{rk.JS.vars} with useful presets. The
#' resulting object can be used inside the dialog XML object (to place the plot options button and disable certain tabs), as
#' well as in the JS object (to then insert the actual plot options).
#'
#' @param label A character string, text label for the button (only used if \code{button=TRUE}).
#' @param embed A character string, registered name (\code{id} in pluginmap file) of the plot options component to be embedded.
#' @param button Logical, whether the plot options should be embedded as a button and appear if it's pressed.
#' @param id.name Character string, a unique ID for this plugin element.
#'		If \code{"auto"}, an ID will be generated automatically from the label and component strings.
#' @return An object of class \code{rk.plot.opts}.
#' @export
#' @seealso \href{help:rkwardplugins}{Introduction to Writing Plugins for RKWard}
#' @examples
#' test.plotOptions <- rk.plotOptions()
#' 
#' # see how differently this object class is treated
#' # e.g., in the XML context
#' rk.XML.dialog(test.plotOptions)
#' # use this in the logic section to disable the "type" slot
#' rk.XML.set(test.plotOptions, set="allow_type", to=FALSE)
#'
#' # now in JS context
#' # manually define the variable
#' cat(rk.paste.JS(test.plotOptions))
#' # this is usually not necessary, as rk.paste.JS.graph() can
#' # define variables automatically
#' cat(
#'   rk.paste.JS.graph(
#'     echo("plot(", test.plotOptions, ")"),
#'     plotOpts=test.plotOptions
#'   )
#' )
#'
#' # as you can also see in the above example, echo() just
#' # fills in the JS varaible
#' echo(test.plotOptions)

rk.plotOptions <- function(label="Generic plot options", embed="rkward::plot_options", button=TRUE, id.name="auto"){

	if(identical(id.name, "auto")){
		id.name <- auto.ids(paste0(embed, label), prefix=ID.prefix("embed"), chars=12)
	} else {}

	genPlotOpts.XML <- rk.XML.embed(component=embed, button=button, label=label, id.name=id.name)
	genPlotOpts.JS.preprocess <- rk.JS.vars(genPlotOpts.XML, modifiers="code.preprocess", check.modifiers=FALSE)
	genPlotOpts.JS.printout <- rk.JS.vars(genPlotOpts.XML, modifiers="code.printout", check.modifiers=FALSE)
	genPlotOpts.JS.calculate <- rk.JS.vars(genPlotOpts.XML, modifiers="code.calculate", check.modifiers=FALSE)

	genPlotOpts.all <- new("rk.plot.opts",
		XML=genPlotOpts.XML,
		preprocess=genPlotOpts.JS.preprocess,
		printout=genPlotOpts.JS.printout,
		calculate=genPlotOpts.JS.calculate)

 	return(genPlotOpts.all)
}
