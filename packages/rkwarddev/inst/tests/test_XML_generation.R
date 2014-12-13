# testing basic tokenizing and POS tagging

# all test standards are stored in a list object called "XML_test_standards"
# it was saved to the file "XML_test_standards.RData"

context("XML")

test_that("about", {
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
      date=as.Date("2014-12-12"),
      url="http://eternalwondermaths.example.org/23/stc.html",
      license="GPL",
      category="Geometry")
  )
  load("XML_test_standards.RData")
  expect_that(
    thisNode, equals(XML_test_standards[["about"]])
  )
})

test_that("attribute", {
  thisNode <- rk.XML.attribute(
    id="testID",
    label="an attribute label",
    value="checked"
  )
  load("XML_test_standards.RData")
  expect_that(
    thisNode, equals(XML_test_standards[["attribute"]])
  )
})

test_that("browser", {
  thisNode <- rk.XML.browser(
    label="a browser label",
    type="dir",
    initial="/tmp/initial",
    filter=c("*initial", "*csv"),
    required=TRUE,
    i18n=list(context="context info here")
  )
  load("XML_test_standards.RData")
  expect_that(
    thisNode, equals(XML_test_standards[["browser"]])
  )
})

test_that("checkbox", {
  thisNode <- rk.XML.cbox(
    label="a checbox label",
    value="checked",
    chk=TRUE,
    un.value="unchecked",
    i18n=list(context="context info here")
  )
  load("XML_test_standards.RData")
  expect_that(
    thisNode, equals(XML_test_standards[["checkbox"]])
  )
})

test_that("code", {
  thisNode <- rk.XML.code(
    file="some_file.js"
  )
  load("XML_test_standards.RData")
  expect_that(
    thisNode, equals(XML_test_standards[["code"]])
  )
})

test_that("col", {
  thisNode <- rk.XML.col(
    rk.XML.cbox(label="a column label")
  )
  load("XML_test_standards.RData")
  expect_that(
    thisNode, equals(XML_test_standards[["col"]])
  )
})

test_that("component", {
  thisNode <- rk.XML.component(
    label="a component label",
    file="plugins/MyGUIdialog.xml",
    i18n=list(context="context info here")
  )
  load("XML_test_standards.RData")
  expect_that(
    thisNode, equals(XML_test_standards[["component"]])
  )
})

test_that("components", {
  thisNode <- rk.XML.components(
    rk.XML.component(
      label="a components label",
      file="plugins/MyOtherGUIdialog.xml"
    )
  )
  load("XML_test_standards.RData")
  expect_that(
    thisNode, equals(XML_test_standards[["components"]])
  )
})

test_that("connect", {
  thisNode <- rk.XML.connect(
    governor="myGovernor",
    client="myCLient",
    get="visible",
    set="string",
    not=TRUE,
    reconcile=TRUE
  )
  load("XML_test_standards.RData")
  expect_that(
    thisNode, equals(XML_test_standards[["connect"]])
  )
})

# test_that("", {
#   thisNode <-
#   load("XML_test_standards.RData")
#   expect_that(
#     thisNode, equals(XML_test_standards[[""]])
#   )
# })
# 
# test_that("", {
#   thisNode <-
#   load("XML_test_standards.RData")
#   expect_that(
#     thisNode, equals(XML_test_standards[[""]])
#   )
# })
# 
# test_that("", {
#   thisNode <-
