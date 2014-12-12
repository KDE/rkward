# testing basic tokenizing and POS tagging

context("XML")

test_that("about", {
  load("about_XML_standard.RData")

  thisNode <- rk.XML.about(
    name="Square the circle",
    author=c(
      person(given="E.A.", family="Dölle",
        email="doelle@eternalwondermaths.example.org", role="aut"),
      person(given="A.", family="Assistant",
        email="alterego@eternalwondermaths.example.org", role=c("cre","ctb"))
      ),
    about=list(
      desc="Squares the circle using Heisenberg compensation.",
      version="0.1-3",
      date=Sys.Date(),
      url="http://eternalwondermaths.example.org/23/stc.html",
      license="GPL",
      category="Geometry")
  )

  expect_that(thisNode,
    equals(thisNodeStandard))
})

# test_that("", {
#   thisNodeStandard <- dget("_XML_dput.txt")
# 
#   thisNode <- rk.XML.()
# 
#   expect_that(thisNode,
#     equals(thisNodeStandard))
# })
