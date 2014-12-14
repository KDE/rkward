\name{NEWS}
\title{News for Package 'XiMpLe'}
\encoding{UTF-8}
\section{Changes in XiMpLe version 0.03-23 (2014-12-14)}{
  \subsection{changed}{
    \itemize{
      \item relocated the unit tests to tests/testthat
    }
  }
}
\section{Changes in XiMpLe version 0.03-22 (2014-12-02)}{
  \subsection{fixed}{
    \itemize{
      \item added curly brackets to the function bodies of all \code{setGeneric()} calls to
        satisfy roxygen2
    }
  }
  \subsection{changed}{
    \itemize{
      \item allowing empty attributes (attr=\code{""}), these were skipped, but only if
        there was more than one attribute
    }
  }
}
\section{Changes in XiMpLe version 0.03-21 (2013-12-21)}{
  \subsection{added}{
    \itemize{
      \item new attribute \code{"as.list"} added to \code{XMLScan()}, if TRUE forces the return
        value to be a list of length 1 (default is still to directly return the
        single node).
      \item new method \code{XMLScanDeep()} recursively scans for attributes and returns all
        which were found
    }
  }
  \subsection{changed}{
    \itemize{
      \item added a proper GPL notice at the beginning of each R file
      \item the vignette was moved from inst/doc to inst/vignettes
      \item synced names of Author: and Maintainer: in DESCRIPTION with Autors@R:
    }
  }
}
\section{Changes in XiMpLe version 0.03-20 (2013-03-30)}{
  \subsection{changed}{
    \itemize{
      \item internally replaced paste(..., sep=\code{""}) calls with paste0(...). for R
        versions older than 2.15 (when \code{paste0()} was introduced), a wrapper function
        should kick in, so we don't need to change the dependencies.
    }
  }
}
\section{Changes in XiMpLe version 0.03-19 (2013-02-24)}{
  \subsection{added}{
    \itemize{
      \item new methods \code{XMLScan()} and \code{XMLScan()}<- to search an XML tree recursively
        for appearances of certain node names
    }
  }
}
\section{Changes in XiMpLe version 0.03-18 (2013-02-11)}{
  \subsection{changed}{
    \itemize{
      \item renamed getter methods from getXML*() into the more generic XML*()
    }
  }
  \subsection{added}{
    \itemize{
      \item added setter methods XML*()<- to all getter methods
      \item added tests for getter/setter methods
    }
  }
}
\section{Changes in XiMpLe version 0.03-17 (2013-02-04)}{
  \subsection{added}{
    \itemize{
      \item added proper \code{is.XiMpLe.node()} and \code{is.XiMpLe.doc()} functions
      \item added getter functions for all slots in XiMpLe.node and XiMpLe.doc
        objects (e.g., \code{getXMLName()} to get the name slot of a XiMpLe.node object)
    }
  }
}
\section{Changes in XiMpLe version 0.03-16 (2012-11-05)}{
  \subsection{added}{
    \itemize{
      \item added examples to all functions
      \item added a vignette
      \item additional tests for \code{node()}<-
    }
  }
  \subsection{fixed}{
    \itemize{
      \item fixed dropping of last tag/text value if XML was incomplete, e.g., just
        an excerpt of a full tree
      \item fixed handling of empty tags completely without space, e.g. "<br/>"
        instead of "<br />"
      \item parsing error for DOCTYPE nodes mixed up elements and previously ignored
        the \code{"decl"} value
      \item \code{node()}<- now replaces both text values and pseudo tags with a new value.
        this can still be problematic for child nodes which contain a mix of
        pseudo and actual tags, so a warning will be given until this is resolved
    }
  }
  \subsection{changed}{
    \itemize{
      \item \code{node()} now reports text values of nodes which have \code{"value"} and a pseudo
        tag at the same time as one character string, like pasteXML does
    }
  }
}
\section{Changes in XiMpLe version 0.03-15 (2012-10-26)}{
  \subsection{fixed}{
    \itemize{
      \item eliminated a memory problem for large XML trees. calling \code{unlist()} with
        use.names=FALSE really makes a difference...
    }
  }
  \subsection{changed}{
    \itemize{
      \item reviewed the garbage collecting and internally moved it out of
        \code{XML.nodes()} into \code{XML.single.tags()}
    }
  }
}
\section{Changes in XiMpLe version 0.03-14 (2012-05-11)}{
  \subsection{added}{
    \itemize{
      \item forcing garbage collection \code{gc()} in internal function \code{XML.nodes()}, which
        iterates through parsed XML trees
    }
  }
  \subsection{changed}{
    \itemize{
      \item better handling of text value indentation via \code{"shine"} and \code{"tidy"}
    }
  }
}
\section{Changes in XiMpLe version 0.03-13 (2012-04-19)}{
  \subsection{added}{
    \itemize{
      \item added a LICENSE file
      \item debianized the sources
    }
  }
  \subsection{changed}{
    \itemize{
      \item \code{parseXMLTree()} now takes connections as \code{"file"} parameter
      \item commented CDATA tags (like they're used in XHTML) can now be detected and
        pasted
      \item \code{pasteXMLTag()} adjusts indentation level of value tags and comments
    }
  }
  \subsection{fixed}{
    \itemize{
      \item feeding internet addresses to \code{parseXMLTree()} no longer throws an error,
        skipping \code{normalize.path()} then now
      \item \code{parseXMLTree()} now preserves newlines in value tags and comments
    }
  }
}
\section{Changes in XiMpLe version 0.03-12 (2012-03-18)}{
  \itemize{
    \item \code{node()} now also works with single XiMpLe.node objects (not only full
      trees)
    \item added pasteXML methods to paste objects of classes XiMpLe.node and
      XiMpLe.doc in a more general manner. the former functions \code{pasteXMLNode()} and
      \code{pasteXMLTree()} are replaced by mere wrapper frunctions to \code{pasteXML()} and
      should not be used any longer.
    \item fixed dropped text value if node also had child nodes in \code{pasteXML()}
    \item fixed missing quotes in DOCTYPE id (\code{pasteXML()})
  }
}
\section{Changes in XiMpLe version 0.03-11 (2012-03-14)}{
  \itemize{
    \item added functions \code{XMLNode()} and \code{XMLTree()} as constructor functions for XML
      nodes and trees.
    \item added opton \code{"object"} to \code{parseXMLTree()}, to be able to parse XML trees not
      only from files, but also character vectors
    \item output of internal function \code{XML.single.tags()} has no colname any more
    \item text values of nodes are no longer followed by an empty newline in
      \code{pasteXMLNode()}
  }
}
\section{Changes in XiMpLe version 0.03-10 (2012-03-11)}{
  \itemize{
    \item fixed bug in parsing of attributes, their names are now all quoted by
      default o not run into issues if they contain R-sensitive characters (like
      "http-equiv")
  }
}
\section{Changes in XiMpLe version 0.03-9 (2012-03-10)}{
  \itemize{
    \item re-formatted the ChangeLog so roxyPackage can translate it into a NEWS.Rd
      file
  }
}
\section{Changes in XiMpLe version 0.03-8 (2011-11-17)}{
  \itemize{
    \item rewrote large parts of internal function \code{XML.single.tags()} for more
      efficiency, allthough it's still quite lethargic when handling huge XML trees
  }
}
\section{Changes in XiMpLe version 0.03-7 (2011-10-23)}{
  \itemize{
    \item added "&" to the special characters for \code{"tidy"} (only applies if theres
      space before and after)
    \item \code{"tidy"} now also indents text in comments and CDATA if it includes the
      newline character
  }
}
\section{Changes in XiMpLe version 0.03-6 (2011-10-09)}{
  \itemize{
    \item added \code{"tidy"} option to the paste functions, which is TRUE by default
      since "<" and ">" are invalid in attributes and text anyway
  }
}
\section{Changes in XiMpLe version 0.03-5 (2011-10-07)}{
  \itemize{
    \item fixed tree parsing bug (in rare cases, comment and CDATA entries caused
      parsing to choke on empty lines)
  }
}
\section{Changes in XiMpLe version 0.03-4 (2011-10-01)}{
  \itemize{
    \item added show methods for objects of class XiMpLe.doc and XiMpLe.node
  }
}
\section{Changes in XiMpLe version 0.03-3 (2011-09-27)}{
  \itemize{
    \item changed default 'shine' value of \code{pasteXMLNode()} and \code{pasteXMLTree()} from 2
      to 1
    \item \code{pasteXMLNode()} and \code{pasteXMLTree()} now try to call each other if fed with
      the wrong class object, but a warning is given
    \item changed handling of 'value' in \code{pasteXMLNode()}, now reflects 'shine'
      setting/indentation
  }
}
\section{Changes in XiMpLe version 0.03-2 (2011-09-17)}{
  \itemize{
    \item added docs for classed XiMpLe.doc and XiMpLe.node
  }
}
\section{Changes in XiMpLe version 0.03-1 (2011-09-11)}{
  \itemize{
    \item fixed outstanding issue with complex value structures
    \item changed "Author@R" to "Authors@R" in DESCRIPTION (for R >= 2.14)
  }
}
\section{Changes in XiMpLe version 0.02-2 (2011-09-01)}{
  \itemize{
    \item fixed missing \code{"methods"} dependecy
    \item adjusted docs a little
  }
}
\section{Changes in XiMpLe version 0.02-1 (2011-08-30)}{
  \itemize{
    \item added option \code{"indent.by"} to the paste functions, in case one prefers
      space over tab.
    \item moved all rk.* functions out of this package and into its own
      (rkwardplugdev)
  }
}
\section{Changes in XiMpLe version 0.01-9 (2011-08-28)}{
  \itemize{
    \item tiny improvements to XML formatting
    \item added functions \code{rk.XML.tabbook()}, \code{rk.XML.dropdown()}, \code{rk.XML.plugin()}
      rk.\code{XML.pluginmap()} and \code{rk.plugin.skeleton()} [to be moved again]
  }
}
\section{Changes in XiMpLe version 0.01-8 (2011-08-26)}{
  \itemize{
    \item improved handling of comments a lot
  }
}
\section{Changes in XiMpLe version 0.01-7 (2011-08-24)}{
  \itemize{
    \item fixed typo bug in \code{XML.single.tags()}
    \item improved support for comments and CDATA
    \item improved XML formatting
  }
}
\section{Changes in XiMpLe version 0.01-6 (2011-08-23)}{
  \itemize{
    \item added functions \code{pasteXMLTree()} and \code{pasteXMLNode()}
    \item added validity checks for classes XiMpLe.doc and XiMpLe.node
    \item added function \code{rk.XML.about()} [which will probably be moved out of the
      package]
  }
}
\section{Changes in XiMpLe version 0.01-5 (2011-08-22)}{
  \itemize{
    \item writing new node values with node<-() partly works
  }
}
\section{Changes in XiMpLe version 0.01-4 (2011-08-21)}{
  \itemize{
    \item rewrote and renamed \code{getNode()} to \code{node()}
  }
}
\section{Changes in XiMpLe version 0.01-3 (2011-08-18)}{
  \itemize{
    \item checks XML and DTD declaration now
    \item added \code{getNode()} method
  }
}
\section{Changes in XiMpLe version 0.01-2 (2011-08-15)}{
  \itemize{
    \item iterating through an XML tree actually works now
  }
}
\section{Changes in XiMpLe version 0.01-1 (2011-08-14)}{
  \itemize{
    \item initial release via reaktanz.de
  }
}
