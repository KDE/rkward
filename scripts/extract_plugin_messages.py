#! /usr/bin/python

BUGADDR = "http://p.sf.net/rkward/bugs"

import codecs
import sys
import os.path
import os
from xml.dom import minidom
import HTMLParser
import copy

def usage ():
  print ("Usage: " + sys.argv[0] + " [--default_po=PO_ID] [--outdir=DIR] files")
  exit (1)

# list of tag-names the content of which to extract in full (including, possibly, HTML-tags, within)
text_containers = ['section', 'text', 'related', 'title', 'summary', 'usage', 'technical', 'setting']
# Elements that refer to a different (labelled) element by id
referring_elements = ['setting', 'caption']

# initialize globals, and parse args
infile = {"infile": "", "file_prefix": "", "caption": "", "id_labels" : {}}
default_po = ""
outfile = ""
outdir = ""
initialized_pot_files = []
current_po_id = ""
toplevel_sources = []
for arg in (list (sys.argv[1:])):
  if (arg.startswith ("--default_po=")):
    default_po = arg.split ("=", 1)[1]
  elif (arg.startswith ("--outdir=")):
    outdir = arg.split ("=", 1)[1]
  elif (arg.startswith ("--")):
    usage ()
  else:
    toplevel_sources.append (arg)
if (len (toplevel_sources) < 1):
  usage ()

# For crying out loud! So we are not strictly using XML, because we allow the use of (X)HTML entities, esp. inside <text>-elements,
# without formally declaring these entities. Python seems to make a point of making it real hard to deal with this. So what we do is
# escaping all entities before parsing, then passing all through HTMLParser.unescape () before writing the output.
def parseFile (filename):
  f = codecs.open (filename, 'r', 'utf-8')
  content = f.read ()
  f.close ()
  return minidom.parseString (content.replace ("&", "&amp;"))

# Where available, include the labels of parent elements. Particularly helpful for radio-options
def getElementShort (element, dot_attribute=""):
  ret = "<" + element.tagName
  for attr in ["label", "title"]:
    if (attr == dot_attribute):
      ret += " " + attr + "=\"...\""
    else:
      if (element.hasAttribute (attr)):
        ret += " " + attr + "=" + quote (element.getAttribute (attr))
  return ret + ">"

# Try to extract helpful file context information
def getFileContext (element, attribute=""):
  ret = "i18n: file: " + infile["infile"] + "\n"
  ret += "i18n: ectx: "
  if (infile["caption"] != ""):
    ret += "(" + infile["caption"] + ") "
  refer_to = ""
  if ((element.tagName in referring_elements) and (element.hasAttribute ("id"))):
    if (not (element.getAttribute ("id") in infile["id_labels"])):
      sys.stderr.write ("WARNING in " + infile["infile"] + ": Reference to unknown element id '" + element.getAttribute ("id") + "'")
    else:
      refer_to = " (refers to element labelled " + quote (infile["id_labels"][element.getAttribute ("id")]) + ")"
  tag_stack = [getElementShort (element, attribute)]
  while ((element.parentNode.nodeType != element.DOCUMENT_NODE)):
    element = element.parentNode
    if (element.tagName in ["document", "row", "column", "frame", "content", "tabbook"]):
      if (not (element.hasAttribute ("label") or element.hasAttribute ("title"))):
        continue # Skip over tags that don't really add any meaningful context information. (Note: <frame>s _with_ a label are meaningful, of course)
    tag_stack.insert (0, getElementShort (element))
  if (len (tag_stack) > 4):
    tag_stack = [tag_stack[0], "[...]"] + tag_stack[-2:]
  return (ret + ' '.join (tag_stack) + refer_to)

def quote (text):
  text = HTMLParser.HTMLParser ().unescape (text)	# unescape character entities, Qt does so while parsing the xml
  return "\"" + text.replace ("\\", "\\\\").replace ("\"", "\\\"") + "\""

# Normalizes larger text fragments. TODO: Do we want to protect <pre>-blocks?
def normalize (text):
  lines = text.split ("\n")
  nlines = []
  for line in lines:
    nlines.append (' '.join (line.strip ().split ()))	# remove whitespace at start, end, and simplify whitespace within each line
  return ' '.join (nlines)
  
# get everything inside the element as text. Might include further xml tags.
def getFullText (element):
  rc = []
  for cn in element.childNodes:
    if cn.nodeType != cn.COMMENT_NODE:
      rc.append(cn.toxml ("utf-8"))
  return ''.join (rc).strip ().replace ("&amp;", "&")

# get the content of all text nodes inside this node (does not include xml tags)
def getText (node):
  rc = []
  for cn in node.childNodes:
    if cn.nodeType in [cn.TEXT_NODE, cn.CDATA_SECTION_NODE]:
      rc.append(cn.data)
  return ''.join (rc).strip ()

# Look for an i18n comment in the given node, and add automatically extracted file context information
def getI18nComment (node, attribute=""):
  ret = "/* "
  for cn in node.childNodes:
    if cn.nodeType == cn.COMMENT_NODE:
      comment = normalize (cn.data.strip ())
      if (comment.lower ().startswith ("i18n:") or comment.lower ().startswith ("translators:")):
        ret += "i18n: " + comment + "\n"
  ret += getFileContext (node, attribute) + " */\n"
  return (ret)

# Main workhorse: Look at given node and recurse into children
def handleNode (node):
  if (node.nodeType == node.ELEMENT_NODE):
    for attr in ["label", "title", "shorttitle"]:
      if (node.hasAttribute (attr)):
        attrv = node.getAttribute (attr)
        if (attrv == ""):
          continue
        outfile.write (getI18nComment (node, attr))
        if (node.hasAttribute ("i18n_context")):
          outfile.write ("i18nc (" + quote (node.getAttribute ("i18n_context")) + ", " + quote (attrv) + ");\n")
        else:
          outfile.write ("i18n (" + quote (attrv) + ");\n")
    if (node.hasAttribute ("file")):
      filename = node.getAttribute ("file")
      if (filename.endswith (".js")):
        filename = os.path.join (os.path.dirname (infile["infile"]), filename)
        jsfile = codecs.open (filename, 'r', 'utf-8')
        handleJSChunk (jsfile.read (), filename, 0, getFileCaption (None, infile["caption"]))
        jsfile.close ()
      else:
        handleSubFile (filename, node.tagName == "component")
    if (node.tagName == "script"):
      handleJSChunk (getText (node), infile["infile"], -1, infile["caption"])
    elif (node.tagName in text_containers):
      textchunks = getFullText (node).split ("\n\n")
      for chunk in textchunks:
        if (chunk != ""):
          outfile.write (getI18nComment (node))
          outfile.write ("i18n (" + quote (normalize (chunk)) + ");\n")
    elif (getText (node) != ""):
      sys.stderr.write ("WARNING: Found text content where none expected: " + getFileContext (node) + "\n")
  if (not ((node.nodeType == node.ELEMENT_NODE) and (node.tagName in text_containers))):
    # Don't go looking into the contents of text containers any further (may contain HTML markup)
    for child in node.childNodes:
      handleNode (child)

# Try to determine a caption for the file (will be used as context comment). If none is found in this file use "Loaded from loaded_from"
def getFileCaption (docelem, loaded_from):
  if (not docelem is None):
    elems = docelem.getElementsByTagName ("title")
    if (elems.length):
      return normalize (getFullText (elems.item (0)))
    elems = docelem.getElementsByTagName ("dialog")
    if (elems.length):
      return elems.item (0).getAttribute ("label")
    elems = docelem.getElementsByTagName ("wizard")
    if (elems.length):
      return elems.item (0).getAttribute ("label")
  if (loaded_from != ""):
    return "Loaded from " + loaded_from
  return ""

# Gather labels of elements with given id (so <setting id="xyz">text</setting> elements can be labelled)
def getElementLabelsRecursive (elem):
  ret = {}
  for ce in elem.childNodes:
    if (ce.nodeType == ce.ELEMENT_NODE):
      if (ce.hasAttribute ("id") and ce.hasAttribute ("label")):
        ret[ce.getAttribute ("id")] = ce.getAttribute ("label")
      ret.update (getElementLabelsRecursive (ce))
  return ret

# It really is sort of lame to have to parse JS and extract i18n-calls, when xgettext could do it. But that would not
# - allow us to add info on which plugin this belongs to
# - list the i18n strings from the JS file in sequence with the i18n strings from the XML parts of the same plugin
# - give decent file context for inlines JS script code
class JSParseBuffer:
  def __init__ (self, content):
    self.buf = content
    self.comment = ""
    self.nline = 0
    self.nchar = 0
  def atEof (self):
    return (self.nchar >= len (self.buf))
  def currentChar (self):
    if (self.atEof ()):
      return ""
    return (self.buf[self.nchar])
  def advance (self, n=1):
    for step in range (n):
      self.nchar += 1
      if (self.nchar >= (len (self.buf) - 1)):
        return False
      if (self.buf[self.nchar] == "\n"):
        self.nline += 1
    return True
  def skipWhitespace (self):
    while (self.buf[self.nchar].isspace ()):
      if (not self.advance ()):
        break
  def seekUntil (self, needle):
    fromchar = self.nchar
    while (not self.startswith (needle)):
      if (not self.advance ()):
        break
    return self.buf[fromchar:self.nchar]
  # returns True, if the given word is found at the current position, _and_ it this is a full keyword (i.e. preceded and followed by some delimiter), and could be a function call
  def isAtFunctionCall (self, word):
    if (not self.startswith (word)):
      return False
    # return False if previous char is no delimiter
    if (self.nchar < 0 and self.buf[self.nchar-1].isalnum ()):
      return False
    # return False if next char is no delimiter
    nextchar = self.buf[self.nchar + len (word)]
    if (not (nextchar.isspace () or nextchar == '(')):
      return False
    return True
  def seek_line_comment_end (self):
    comment = ""
    while True:
      comment += self.seekUntil ("\n")
      self.skipWhitespace ()    
      if (self.startswith ("//")):
        self.advance (2)
      else:
        break
    return comment
  def nibbleCallParameters (self):
    fromchar = self.nchar
    self.nibble_until ('(')
    self.advance ()
    self.nibble_until (')', True)
    self.advance ()
    return self.buf[fromchar:self.nchar]
  # TODO: handle includes, somehow
  def nibble_until (self, string, skip_over_parentheses=False):
    fromchar = self.nchar
    while (not self.startswith (string)):
      if (self.atEof ()):
        break
      if (self.startswith ('\\')):
        self.advance (2)
        continue
      if (self.buf[self.nchar] in ['"', '\'', '`']):
        ochar = self.buf[self.nchar]
        if (self.advance ()):
          self.seekUntil (ochar)
      elif (self.startswith ("/*")):
        self.comment = self.seekUntil ("*/")
      elif (self.startswith ("//")):
        self.comment = self.seek_line_comment_end ()
      elif (skip_over_parentheses and (self.startswith ("("))):	# skip over nested parentheses
        self.advance ()
        self.nibble_until (")", True)
      elif (not self.buf[self.nchar].isspace ()):
        self.comment = ""
      if (not self.advance ()):
        break
    return (self.buf[fromchar:self.nchar])
  def startswith (self, string):
    return self.buf.startswith (string, self.nchar)

def handleJSChunk (buf, filename, offset, caption):
  global outfile

  jsbuf = JSParseBuffer (buf)
  while (True):
    call = ""
    junk = jsbuf.nibble_until ("i18n")
    if (jsbuf.atEof ()):
      break
    for candidate in ["i18n", "i18nc", "i18np", "i18ncp"]:
      if (jsbuf.isAtFunctionCall (candidate)):
        print ("found " + candidate)
        call = candidate
        break
    if (call == ""):
      # skip over somethingelsei18nsomethingelse identifiers, i.e. those not matched, above
      jsbuf.advance ()
      continue

    jsbuf.advance (len (call))
    comment = normalize (jsbuf.comment)
    line = jsbuf.nline
    parameters = jsbuf.nibbleCallParameters ()
    # Ok, here's another crude hack... Strip "noquote()" from anything inside the i18n-call, as xgettext will ignore strings inside other calls
    subbuf = JSParseBuffer (parameters)
    parameters = ""
    while (not subbuf.atEof ()):
      parameters += subbuf.nibble_until ("noquote")
      if (subbuf.isAtFunctionCall ("noquote")):
        subbuf.advance (len ("noquote"))
        parameters += subbuf.nibbleCallParameters ().strip ()[1:][:-1]	# strip parentheses, too
    # Hack end.
    text = "/* "
    if (comment.lower ().startswith ("i18n:") or comment.lower ().startswith ("translators:")):
      text += "i18n: " + comment + "\n"
    text += "i18n: file: " + filename
    if (offset >= 0):
      text += ":" + str (offset + line + 1)
    text += "\ni18n: ectx: (" + caption + ") */\n"
    text += call + parameters + ";\n"
    outfile.write (text)

# When we encounter a "file"-attribute, we generally dive right into parsing that file, i.e. we do depth first
# Advantage is that strings in all files belonging to one plugin will be in direct succession in the .pot file
# The exception is if the referenced file declares an own (different) po_id. In this case it will be handled, later.
def handleSubFile (filename, fetch_ids = False):
  global toplevel_sources
  global infile
  cdir = os.path.dirname (infile["infile"])
  filename = os.path.join (cdir, infile["file_prefix"], filename)
  if (not os.path.isfile (filename)):
    sys.stderr.write (getFileContext (node)  + " WARNING: File " + filename + " does not exist\n")
    return
  xmldoc = parseFile (filename)
  if (xmldoc.documentElement.hasAttribute ("po_id") and (xmldoc.documentElement.getAttribute ("po_id") != current_po_id)):
    toplevel_sources.append (filename)
    #sys.stderr.write ("Added " + filename + " to toplevel\n")
  else:
    #sys.stderr.write ("Recursing into " + filename + "\n")
    oldinfile = copy.deepcopy (infile)
    infile["infile"] = filename
    infile["file_prefix"] = xmldoc.documentElement.getAttribute ("base_prefix")
    infile["caption"] = getFileCaption (xmldoc.documentElement, oldinfile["caption"])
    if (fetch_ids):
      infile["id_labels"] = getElementLabelsRecursive (xmldoc.documentElement)
    handleNode (xmldoc.documentElement)
    infile = oldinfile

def initialize_pot_file (po_id):
  global outfile
  global current_po_id
  current_po_id = po_id
  if (outfile != ""):
    outfile.close ()
  if (current_po_id in initialized_pot_files):
    mode = 'a'
  else:
    initialized_pot_files.append (current_po_id)
    mode = 'w'
  outfile = codecs.open (os.path.join (outdir, po_id + '.pot.cpp'), mode, 'utf-8')

#######
# Loop over toplevel_sources (specified on command line, or those that want to be split into separate po) and extract messages
# NOTE: toplevel_sources may grow, dynamically, but only at the end.
i = 0
while i < len (toplevel_sources):
  xmldoc = parseFile (toplevel_sources[i])
  po_id = xmldoc.documentElement.getAttribute ("po_id")
  if (po_id == ""):
    po_id = default_po
  if (po_id == ""):
    sys.stderr.write ("WARNING: No po_id attribute on file " + toplevel_sources[i] + " and no default specified. Skipping.\n")
    continue
  initialize_pot_file (po_id)
  handleSubFile (toplevel_sources[i])  # Some duplication of parsing, instead of duplication of code
  i += 1

#######
# Run xgettext on all generated .pot.cpp files
for potcpp in initialized_pot_files:
  potcppfile = os.path.join (outdir, potcpp + ".pot.cpp")
  # NOTE: using --no-location, as that just adds meaningless references to the temporary .pot.cpp-file.
  os.system ("xgettext --from-code=UTF-8 -C -kde -ci18n -ki18n:1 -ki18nc:1c,2 -ki18np:1,2 -ki18ncp:1c,2,3 -ktr2i18n:1 " + 
             "-kI18N_NOOP:1 -kI18N_NOOP2:1c,2 -kaliasLocale -kki18n:1 -kki18nc:1c,2 -kki18np:1,2 -kki18ncp:1c,2,3 --no-location " +
             "--msgid-bugs-address=" + BUGADDR + " -o " + os.path.join (outdir, "rkward__" + potcpp + ".pot ") + potcppfile)
  os.remove (potcppfile)
