\name{rk.show.message}
\alias{rk.show.message}
\alias{rk.show.question}

\title{Message box using native KDE GUI}

\description{
  Multi-purpose pop-up message boxes using native KDE GUI elements; used either to show some information or ask some question.
}

\usage{
rk.show.message (message, caption = "Information", wait = TRUE)

rk.show.question (message, caption = "Question", 
                   button.yes = "yes", button.no = "no", button.cancel = "cancel")
}

\arguments{
  \item{message}{a string for the content of the message box.}
  \item{caption}{a string for title of the message box.}
  \item{button.yes}{a string for the text label of the \bold{Yes} button. Can be an empty string (\code{""}), in which case the button is not displayed at all.}
  \item{button.no}{a string used for the text label of the \bold{No} button, similar to \code{button.yes}.}
  \item{button.canel}{a string used for the text label of the \bold{Cancel} button, similar to \code{button.yes}.}
  \item{wait}{a logical (not NA) indicating whether the R interpreter should wait for the user's action, or run it asynchronously.}
}

\details{
  For \code{rk.show.question}, the R interpreter always waits for the user's choice.
}

\value{
  \code{rk.show.message} always returns \code{TRUE}, invisibly.
  
  \code{rk.show.question} returns \code{TRUE} for \bold{Yes}, \code{FALSE} for \bold{No}, and \code{NULL} for \bold{Cancel} actions.
}

\author{Thomas Friedrichsmeier \email{rkward-devel@lists.sourceforge.net}}

\seealso{
  \code{\link{system}}
}
\examples{
require (rkward)

if (rk.show.question ("Question:\nDo you want to know about RKWard?", 
    button.yes = "Yes, I do!", button.no = "No, I don't care!", button.cancel = "")) {
  rk.show.message ("Message:\nRKWard is a KDE GUI for R.", "RKWard Info")
} else {
  rk.show.message ("You must be joking!", "RKWard Info", wait = FALSE) ## Run asynchronously
}
}

\keyword{utilities}
