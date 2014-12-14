# testing basic tokenizing and POS tagging

# all test standards are stored in a list object called "XML_test_standards"
# it was saved to the file "XML_test_standards.RData"

context("XML")

test_that("rk.XML.about", {
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

test_that("rk.XML.attribute", {
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

test_that("rk.XML.browser", {
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

test_that("rk.XML.cbox", {
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

test_that("rk.XML.code", {
  thisNode <- rk.XML.code(
    file="some_file.js"
  )
  load("XML_test_standards.RData")
  expect_that(
    thisNode, equals(XML_test_standards[["code"]])
  )
})

test_that("rk.XML.col", {
  thisNode <- rk.XML.col(
    rk.XML.cbox(label="a column label")
  )
  load("XML_test_standards.RData")
  expect_that(
    thisNode, equals(XML_test_standards[["col"]])
  )
})

test_that("rk.XML.component", {
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

test_that("rk.XML.components", {
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

test_that("rk.XML.connect", {
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

test_that("rk.XML.context", {
  thisNode <- rk.XML.context(
    rk.XML.menu("Analysis", 
      rk.XML.entry(
        rk.XML.component(
          label="a context label",
          file="plugins/MyOtherGUIdialog.xml"
        )
      )
    )
  )
  load("XML_test_standards.RData")
  expect_that(
    thisNode, equals(XML_test_standards[["context"]])
  )
})

test_that("rk.XML.convert", {
  thisNode <- rk.XML.convert(
    sources=list(text=rk.XML.input("a convert label")),
    mode=c(equals="myValue"),
    required=TRUE
  )
  load("XML_test_standards.RData")
  expect_that(
    thisNode, equals(XML_test_standards[["convert"]])
  )
})

test_that("rk.XML.copy", {
  thisNode <- rk.XML.copy(
    id="aCopyID",
    as="page"
  )
  load("XML_test_standards.RData")
  expect_that(
    thisNode, equals(XML_test_standards[["copy"]])
  )
})

test_that("rk.XML.dependencies", {
  thisNode <- rk.XML.dependencies(
    dependencies=list(
      rkward.min="0.6.3",
      rkward.max="0.6.5",
      R.min="3.1",
      R.max="3.2"),
    package=list(
      c(name="heisenberg", min="0.11-2", max="0.14-1",
        repository="http://hsb.example.org"),
      c(name="DreamsOfPi", min="0.2", max="3.1", repository="http://dop.example.org")),
    pluginmap=list(
      c(name="heisenberg.pluginmap", url="http://eternalwondermaths.example.org/hsb"))
  )
  load("XML_test_standards.RData")
  expect_that(
    thisNode, equals(XML_test_standards[["dependencies"]])
  )
})

test_that("rk.XML.dependency_check", {
  thisNode <- rk.XML.dependency_check(
    id.name="dependency",
    dependencies=list(
      rkward.min="0.6.3",
      rkward.max="0.6.5",
      R.min="3.1",
      R.max="3.2"),
    package=list(
      c(name="heisenberg", min="0.11-2", max="0.14-1",
        repository="http://hsb.example.org"),
      c(name="DreamsOfPi", min="0.2", max="3.1", repository="http://dop.example.org")),
    pluginmap=list(
      c(name="heisenberg.pluginmap", url="http://eternalwondermaths.example.org/hsb"))
  )
  load("XML_test_standards.RData")
  expect_that(
    thisNode, equals(XML_test_standards[["dependency_check"]])
  )
})

test_that("rk.XML.dialog", {
  thisNode <- rk.XML.dialog(
    rk.XML.col(
      rk.XML.cbox(label="a dialog column label")
    ),
    label="a dialog label",
    recommended=TRUE,
    i18n=list(context="context info here")
  )
  load("XML_test_standards.RData")
  expect_that(
    thisNode, equals(XML_test_standards[["dialog"]])
  )
})

test_that("rk.XML.dropdown", {
  thisNode <- rk.XML.dropdown(
    label="myDropdownMenu",
    options=list(
      "First Option"=c(val="val1"),
      rk.XML.option(
        "Second Option", val="val2", id.name="auto",
        i18n=list(context="context info1 here")
      ),
      "Third Option"=c(val="val3", chk=TRUE)
    ),
    i18n=list(context="context info2 here")
  )
  load("XML_test_standards.RData")
  expect_that(
    thisNode, equals(XML_test_standards[["dropdown"]])
  )
})

# test_that("", {
#   thisNode <-
#   load("XML_test_standards.RData")
#   expect_that(
#     thisNode, equals(XML_test_standards[[""]])
#   )
# })
