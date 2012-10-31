\name{rk.get.tempfile.name}
\alias{rk.flush.output}
\alias{rk.get.output.html.file}
\alias{rk.get.tempfile.name}
\alias{rk.get.workspace.url}
\alias{rk.set.output.html.file}
\title{RKWard file names}
\usage{
  rk.get.tempfile.name(prefix = "image",
    extension = ".jpg")

  rk.get.workspace.url()

  rk.get.output.html.file()

  rk.set.output.html.file(x,
    additional.header.contents = getOption("rk.html.header.additions"))

  rk.flush.output(x = rk.get.output.html.file(),
    ask = TRUE)
}
\arguments{
  \item{prefix}{a string, used as a filename prefix when
  saving images to the output file}

  \item{extension}{a string, used as a filename extension
  when saving images to the output file}

  \item{x}{a string, giving the filename of the of the
  output file}

  \item{additional.header.contents}{NULL or an additional
  string to add to the HTML header section.  This could be
  scripts or additional CSS definitions, for example. Note
  that}

  \item{ask}{Logical: Whether to ask before flushing the
  output file.}
}
\value{
  \code{rk.get.tempfile.name},
  \code{rk.get.output.html.file}, and
  \code{rk.get.workspace.url} return a string while
  \code{rk.set.output.html.file} returns \code{NULL}.
}
\description{
  In RKWard the output is saved as a html file which is
  located at "~/.rkward" by default. (\bold{TODO}: make
  this platform free). The name of this html file can be
  retrieved and set using \code{rk.get.output.html.file}
  and \code{rk.set.output.html.file}.
  \code{rk.flush.output.html.file} will delete the current
  (or specified) html file, and re-initialize it.
}
\details{
  \code{rk.get.tempfile.name} returns a non-existing
  filename inside the directory of the output file. It is
  mainly used by \link{rk.graph.on} to create filenames
  suitable for storing images in the output. The filenames
  of the temporary files are of the form
  "\code{prefix}\emph{xyz}.\code{extension}".
  \code{rk.get.tempfile.name} is somewhat misnamed. For
  truly temporary files, \link{tempfile} is generally more
  suitable.

  \code{rk.get.workspace.url} returns the url of workspace
  file which has been loaded in RKWard, or NULL, if no
  workspace has been loaded. NOTE: This value is note
  affected by running \code{load} in R, only by loading R
  workspaces via the RKWard GUI.
}
\examples{
testfile.name <- rk.get.tempfile.name(prefix="test", extension=".txt")
testfile <- file(testfile.name)
cat("This is a test\\n", file=testfile)
close(testfile)
unlink(testfile.name)

outfile <- rk.get.output.html.file()

## Not run
rk.set.output.html.file("~/.rkward/another_file.html")
rk.header("Output on a different output file")
rk.show.html(rk.get.output.html.file())
rk.set.output.html.file(outfile)
}
\author{
  Thomas Friedrichsmeier
  \email{rkward-devel@lists.sourceforge.net}
}
\seealso{
  \url{rkward://page/rkward_output}, \link{tempfile},
  \link{file}, \link{rk.print}
}
\keyword{IO}
\keyword{utilities}

