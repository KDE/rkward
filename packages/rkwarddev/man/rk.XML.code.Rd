\name{rk.XML.code}
\alias{rk.XML.code}
\title{Create XML node "code" for RKWard plugins}
\usage{
  rk.XML.code(file)
}
\arguments{
  \item{file}{A character string, the JavaScript file name
  to be included.}
}
\value{
  An object of class \code{XiMpLe.node}.
}
\description{
  Create XML node "code" for RKWard plugins
}
\examples{
test.code <- rk.XML.code("some_file.js")
cat(pasteXML(test.code))
}
\seealso{
  \href{help:rkwardplugins}{Introduction to Writing Plugins
  for RKWard}
}

