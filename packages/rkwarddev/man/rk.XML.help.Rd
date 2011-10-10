\name{rk.XML.help}
\alias{rk.XML.help}
\title{Create XML node "help" for RKWard plugins}
\usage{
  rk.XML.help(file)
}
\arguments{
  \item{file}{A character string, the file name to be
  included as reference.}
}
\value{
  An object of class \code{XiMpLe.node}.
}
\description{
  Create XML node "help" for RKWard plugins
}
\examples{
test.help <- rk.XML.help("some_file.rkh")
cat(pasteXMLNode(test.help))
}
\seealso{
  \href{help:rkwardplugins}{Introduction to Writing Plugins
  for RKWard}
}

