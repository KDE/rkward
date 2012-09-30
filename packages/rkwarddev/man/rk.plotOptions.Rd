\name{rk.plotOptions}
\alias{rk.plotOptions}
\title{Create an object for plot options in RKWard plugins}
\usage{
  rk.plotOptions(label = "Generic plot options",
    embed = "rkward::plot_options", button = TRUE,
    id.name = "auto")
}
\arguments{
  \item{label}{A character string, text label for the
  button (only used if \code{button=TRUE}).}

  \item{embed}{A character string, registered name
  (\code{id} in pluginmap file) of the plot options
  component to be embedded.}

  \item{button}{Logical, whether the plot options should be
  embedded as a button and appear if it's pressed.}

  \item{id.name}{Character string, a unique ID for this
  plugin element.  If \code{"auto"}, an ID will be
  generated automatically from the label and component
  strings.}
}
\value{
  An object of class \code{rk.plot.opts}.
}
\description{
  Generates XML and JavaScript code snippets by calling
  \code{rk.XML.embed} and \code{rk.JS.vars} with useful
  presets. The resulting object can be used inside the
  dialog XML object (to place the plot options button and
  disable certain tabs), as well as in the JS object (to
  then insert the actual plot options).
}
\examples{
test.plotOptions <- rk.plotOptions()

# see how differently this object class is treated
# e.g., in the XML context
rk.XML.dialog(test.plotOptions)
# use this in the logic section to disable the "type" slot
rk.XML.set(test.plotOptions, set="allow_type", to=FALSE)

# now in JS context
# manually define the variable
cat(rk.paste.JS(test.plotOptions))
# this is usually not necessary, as rk.paste.JS.graph() can
# define variables automatically
cat(
  rk.paste.JS.graph(
    echo("plot(", test.plotOptions, ")"),
    plotOpts=test.plotOptions
  )
)

# as you can also see in the above example, echo() just
# fills in the JS varaible
echo(test.plotOptions)
}
\seealso{
  \href{help:rkwardplugins}{Introduction to Writing Plugins
  for RKWard}
}

