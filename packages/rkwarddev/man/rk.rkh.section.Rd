\name{rk.rkh.section}
\alias{rk.rkh.section}
\title{Create XML "section" node for RKWard help pages}
\usage{
  rk.rkh.section(title, text = NULL, short = NULL, id.name
  = "auto")
}
\arguments{
  \item{title}{Character string, title to be displayed.}

  \item{text}{Character string, the text to be displayed.}

  \item{short}{Character string, short title for the menu
  for links to this section.}

  \item{id.name}{Character string, a unique ID for this
  element. If \code{"auto"}, an ID will be generated
  automatically from the \code{title} value.}
}
\value{
  An object of class \code{XiMpLe.node}.
}
\description{
  This function will create a section node for settings
  sections in RKWard help files.
}
\examples{
test.section <- rk.rkh.section("General background", text="Some important notes...",
short="Background")
cat(pasteXMLNode(test.section))
}
\seealso{
  \code{\link[rkwarddev:rk.rkh.doc]{rk.rkh.doc}} and the
  \href{help:rkwardplugins}{Introduction to Writing Plugins
  for RKWard}
}

