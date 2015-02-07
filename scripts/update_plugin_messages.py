#! /usr/bin/python
# ***************************************************************************
#                          update_plugin_messages  -  description
#                             -------------------
#    begin                : Oct 2014
#    copyright            : (C) 2014 by Thomas Friedrichsmeier
#    email                : tfry@users.sourceforge.net
# ***************************************************************************
#
# ***************************************************************************
# *                                                                         *
# *   This program is free software; you can redistribute it and/or modify  *
# *   it under the terms of the GNU General Public License as published by  *
# *   the Free Software Foundation; either version 2 of the License, or     *
# *   (at your option) any later version.                                   *
# *                                                                         *
# ***************************************************************************
#
# Extracts messages form RKWard plugin files (.pluginmap, .xml, .rkh, .js).
# Unless --extract-only is specified on the command line, also merges existing
# translations with the message template, compiles them, and installs them.
#
# Included files are walked, automatically. Thus the typical usage is to specify
# the topmost .pluginmap file as the only file argument.

import os
import codecs
import sys
import subprocess
from xml.dom import minidom
import HTMLParser
import copy
import re

# You might want to adjust the following values (can also be overridden from environment variable):
BUGADDR = "http://p.sf.net/rkward/bugs"     # Technically, this is for bugs _in the translation_
BUGADDR = os.getenv ('BUGADDR', BUGADDR)
XGETTEXT_CALL =  "xgettext --from-code=UTF-8 -C -kde -ci18n -ki18n:1 -ki18nc:1c,2 -ki18np:1,2 -ki18ncp:1c,2,3"
XGETTEXT_CALL += " -ktr2i18n:1 -kI18N_NOOP:1 -kI18N_NOOP2:1c,2 -kaliasLocale -kki18n:1 -kki18nc:1c,2 -kki18np:1,2"
XGETTEXT_CALL += " -kki18ncp:1c,2,3 --msgid-bugs-address=" + BUGADDR
XGETTEXT_CALL = os.getenv ('XGETTEXT', XGETTEXT_CALL)
MSGMERGE = os.getenv ('MSGMERGE', "msgmerge")
MSGFMT = os.getenv ('MSGFMT', "msgfmt --check")
# end

# list of tag-names the content of which to extract in full (including, possibly, HTML-tags, within)
text_containers = ['section', 'text', 'related', 'title', 'summary', 'usage', 'technical', 'setting']
# (HTML)-elements on which to split translation units within a text_container
text_splitting_elements = ['p', 'ul', 'ol', 'li']
# Elements that refer to a different (labelled) element by id
referring_elements = ['setting', 'caption']
# Map of elements to attributes to extract, and default context info
attributes_to_extract_for_tag={
  '*':      { "attributes" : ['label', 'title', 'shorttitle'], "context": ''},
  'about':  { "attributes" : ['name', 'shortinfo', 'category'], "context": ''},
  'author': { "attributes" : ['name', 'given', 'family'], "context": 'Author name'}
}
# HACK for preserving line number information in the DOM tree. This string should be unique enough to not clash with the files' contents!
LINE_DUMMY_ATTR = '_DUMMY_LINE'

def usage ():
  print ("Usage: " + sys.argv[0] + " [--default_po=PO_ID] [--outdir=DIR] files")
  exit (1)

# initialize globals, and parse args
infile = {"infile": "", "file_prefix": "", "caption": "", "id_labels" : {}}
default_po = ""
outfile = ""
outdir = ""
initialized_pot_files = []
po_file_install_locations = {}
current_po_id = ""
do_merge_install = True
toplevel_sources = []
for arg in (list (sys.argv[1:])):
  if (arg.startswith ("--default_po=")):
    default_po = arg.split ("=", 1)[1]
  elif (arg.startswith ("--outdir=")):
    outdir = arg.split ("=", 1)[1]
  elif (arg == ("--extract-only")):
    do_merge_install = False
  elif (arg.startswith ("--")):
    usage ()
  else:
    toplevel_sources.append (arg)
if (len (toplevel_sources) < 1):
  usage ()

# For crying out loud! So we are not strictly using XML, because we allow the use of (X)HTML entities, esp. inside <text>-elements,
# without formally declaring these entities. Python seems to make a point of making it real hard to deal with this. So what we do is
# escaping all entities before parsing, then passing all through HTMLParser.unescape () before writing the output.
#
# The second thing we do is hacking line number information into the parsed XML tree
def parseFile (filename):
  f = codecs.open (filename, 'r', 'utf-8')
  l = 0
  enriched = list ()
  for line in f:
    l += 1
    enriched.append (re.sub (r'<(\w+)', r'<\1 ' + LINE_DUMMY_ATTR + '="' + str (l) + '"', line))
  content = "".join (enriched).replace ("&", "&amp;")
  f.close ()

  try:
    return minidom.parseString (content.encode ('utf-8'))
  except:
    sys.stderr.write ("ERROR: Failed to parse file " + filename + "\n")
    raise

# Where available, include the labels of parent elements. Particularly helpful for radio-options
def getElementShort (element, dot_attribute=""):
  ret = "<" + element.tagName
  if (dot_attribute != ""):
    ret += " " + dot_attribute + "=\"...\""
  else:
    for attr in ["label", "title"]:
      if (element.hasAttribute (attr)):
        ret += " " + attr + "=" + quote (element.getAttribute (attr))
  return ret + ">"

# Try to extract helpful file context information
def getFileContext (element, attribute=""):
  ret = "i18n: file: " + infile["infile"] + ":" + str (getLineOf (element, 0)) + "\n"
  ret += "i18n: ectx: "
  if (infile["caption"] != ""):
    ret += "(" + infile["caption"] + ") "
  refer_to = ""
  if ((element.tagName in referring_elements) and (element.hasAttribute ("id"))):
    if (not (element.getAttribute ("id") in infile["id_labels"])):
      sys.stderr.write ("WARNING in " + infile["infile"] + ", line " + str (getLineOf (element)) + ": Reference to unknown (or unnamed) element id '" + element.getAttribute ("id") + "'\n")
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

def writeouti18n (call):
  if (call.find ("%") >= 0):
    outfile.write ("/* xgettext:no-c-format */ ")
  outfile.write (call + "\n")

def quote (text):
  try:
     text = text.decode ('utf-8', 'ignore')
  except:
     print ("Python has trouble decoding this text: " + text.encode('utf-8'))
  text = HTMLParser.HTMLParser ().unescape (text)	# unescape character entities, Qt does so while parsing the xml
  return "\"" + text.replace ("\\", "\\\\").replace ("\"", "\\\"") + "\""

def stripLineDummy (text):
  return re.sub (r' ' + LINE_DUMMY_ATTR + '="\d+"', '', text)

def getLineOf (element, default=-1):
  if element.hasAttribute (LINE_DUMMY_ATTR):
    return int (element.getAttribute (LINE_DUMMY_ATTR))
  return default

# Normalizes larger text fragments. TODO: Do we want to protect <pre>-blocks?
def normalize (text):
  lines = text.split ("\n")
  nlines = []
  for line in lines:
    nlines.append (' '.join (line.strip ().split ()))	# remove whitespace at start, end, and simplify whitespace within each line
  return ' '.join (nlines)
  
# get everything inside the element as text. Might include further xml tags.
def getFullText (element):
  childnodes = element.childNodes
  ## Skip over anything containing only a <link href="rkward://"/> and nothing else; a somewhat important special case ("Related"-section)
  ## NOTE: The second attribute, here, is the LINE_DUMMY_ATTR , hence checking for cn.attributes.length == 2
  if (childnodes.length == 1):
    cn = childnodes.item (0)
    if (not cn.hasChildNodes()) and (cn.nodeType == cn.ELEMENT_NODE) and (cn.tagName == "link") and (cn.attributes.length == 2) and (cn.getAttribute ("href").startswith ("rkward://")):
      return ""

  rc = []
  for cn in childnodes:
    if (cn.nodeType == cn.ELEMENT_NODE) and (cn.tagName in text_splitting_elements):
      rc.append ("\n\n" + getFullText (cn) + "\n\n")
    elif cn.nodeType != cn.COMMENT_NODE:
      rc.append(stripLineDummy (cn.toxml ("utf-8")))
  return ''.join (rc).strip ().replace ("&amp;", "&")

# get the content of all text nodes inside this node (does not include xml tags)
def getText (node):
  rc = []
  for cn in node.childNodes:
    if cn.nodeType in [cn.TEXT_NODE, cn.CDATA_SECTION_NODE]:
      rc.append(stripLineDummy (cn.data))
  return ''.join (rc).strip ()

# Look for an i18n comment in the given node, and add automatically extracted file context information
def getI18nComment (node, attribute=""):
  ret = "/* "
  for cn in node.childNodes:
    if cn.nodeType == cn.COMMENT_NODE:
      comment = normalize (cn.data.strip ())
      if (comment.lower ().startswith ("i18n:") or comment.lower ().startswith ("translators:")):
        ret += "i18n: " + stripLineDummy (comment) + "\n"
  ret += getFileContext (node, attribute) + " */\n"
  return (ret)

# Main workhorse: Look at given node and recurse into children
def handleNode (node):
  if (node.nodeType == node.ELEMENT_NODE):
    attributes = attributes_to_extract_for_tag['*']['attributes']
    context = attributes_to_extract_for_tag['*']['context']
    if (node.tagName in attributes_to_extract_for_tag):
      attributes = attributes + attributes_to_extract_for_tag[node.tagName]['attributes']
      context = attributes_to_extract_for_tag[node.tagName]['context']
    for attr in attributes:
      if (node.hasAttribute (attr)):
        attrv = node.getAttribute (attr)
        if (attrv == ""):
          continue
        outfile.write (getI18nComment (node, attr))
        if (node.hasAttribute ("i18n_context")):
          context = node.getAttribute ("i18n_context")
        if (context != ''):
          writeouti18n ("i18nc (" + quote (context) + ", " + quote (attrv) + ");")
        else:
          writeouti18n ("i18n (" + quote (attrv) + ");")
    if (node.hasAttribute ("file")):
      filename = node.getAttribute ("file")
      if (filename.endswith (".js")):
        filename = os.path.join (os.path.dirname (infile["infile"]), filename)
        jsfile = codecs.open (filename, 'r', 'utf-8')
        handleJSChunk (jsfile.read (), filename, 0, getFileCaption (None, infile["caption"]))
        jsfile.close ()
      else:
        handleSubFile (filename, node.tagName == "component", node.tagName == "include")
    if (node.tagName == "script"):
      handleJSChunk (getText (node), infile["infile"], getLineOf (node), infile["caption"])
    elif (node.tagName in text_containers):
      textchunks = getFullText (node).split ("\n\n")
      for chunk in textchunks:
        chunk = chunk.strip ()
        if (chunk != ""):
          outfile.write (getI18nComment (node))
          writeouti18n ("i18n (" + quote (normalize (chunk)) + ");")
    elif (getText (node) != ""):
      sys.stderr.write ("WARNING: Found text content where none expected: " + getFileContext (node) + "\n")
      sys.stderr.write (quote (getText (node)))
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

def getAllElementLabels (xmldoc, filename):
  ret = getElementLabelsRecursive (xmldoc)
  includes = xmldoc.getElementsByTagName ("include")
  for inc in includes:
    subfile = os.path.join (os.path.dirname (filename), inc.getAttribute ("file"))
    subdoc = parseFile (subfile)
    ret.update (getAllElementLabels (subdoc, subfile))
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
      if (self.buf[self.nchar] in ['"', '\'', '`']):
        ochar = self.buf[self.nchar]
        while (self.advance ()):
          if (self.startswith ('\\')):
            self.advance ()	# skip next char
          elif (self.startswith (ochar)):
            break
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

  # Convert single quoted and backtick quoted strings in the input chunk to double quotes (so xgettext can work in C-style).
  def normalizeQuotes (chunk):
    pos = 0
    current_quote_symbol = ""
    output = ""
    strip_closing_parentheses = 0
    while (pos < len (chunk)):
      c = chunk[pos]
      if c == "\\":
        nc = chunk[pos+1]
        if ((nc != current_quote_symbol) or (nc == '"')):
          output += c
        output += nc
        pos += 1
      elif c in ['"', '\'', '`']:
        if (current_quote_symbol == ""):
          current_quote_symbol = c
          output += '"'
        elif current_quote_symbol == c:
          current_quote_symbol = ""
          output += '"'
        elif c == '"':
          output += '\\\"'
        else:
          output += c
      else:
        output += c
      pos += 1
    return output

  jsbuf = JSParseBuffer (buf)
  while (True):
    call = ""
    junk = jsbuf.nibble_until (("i18n", "comment"))
    if (jsbuf.atEof ()):
      break
    for candidate in ["i18n", "i18nc", "i18np", "i18ncp", "comment"]:
      if (jsbuf.isAtFunctionCall (candidate)):
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
    if (call == "comment"):
      call = "i18nc" # for xgettext
      parameters = parameters.replace ('(', '("R code comment", ', 1)
    text += call + normalizeQuotes (parameters) + ";\n"
    writeouti18n (text)

# When we encounter a "file"-attribute, we generally dive right into parsing that file, i.e. we do depth first
# Advantage is that strings in all files belonging to one plugin will be in direct succession in the .pot file
# The exception is if the referenced file declares an own (different) po_id. In this case it will be handled, later.
def handleSubFile (filename, fetch_ids = False, is_include=False):
  global toplevel_sources
  global infile
  cdir = os.path.dirname (infile["infile"])
  if (is_include):
    filename = os.path.join (cdir, filename)
  else:
    filename = os.path.join (cdir, infile["file_prefix"], filename)
  if (not os.path.isfile (filename)):
    sys.stderr.write (" WARNING: File " + filename + " (referenced from " + infile["infile"] + ") does not exist\n")
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
      infile["id_labels"] = getAllElementLabels (xmldoc.documentElement, filename)
    handleNode (xmldoc.documentElement)
    infile = oldinfile

def initialize_pot_file (po_id, po_loc):
  global outfile
  global current_po_id
  current_po_id = po_id
  if (outfile != ""):
    outfile.close ()
  if (current_po_id in initialized_pot_files):
    if (po_file_install_locations[current_po_id] != po_loc):
      sys.stderr.write ("WARNING: Conflicting path specs for po id " + po_id)
    mode = 'a'
  else:
    initialized_pot_files.append (current_po_id)
    po_file_install_locations[current_po_id] = po_loc
    mode = 'w'
  p_outdir = outdir
  if (p_outdir == ""):
    p_outdir = po_file_install_locations[po_id]
  if (not os.path.exists (p_outdir)):
    os.makedirs (p_outdir, 0755)
  outfile = codecs.open (os.path.join (p_outdir, po_id + '.pot.cpp'), mode, 'utf-8')
  if (mode == 'w'):	# just created
    outfile.write ('i18nc("NAME OF TRANSLATORS","Your names");\n');
    outfile.write ('i18nc("EMAIL OF TRANSLATORS","Your emails");\n');

#######
# Loop over toplevel_sources (specified on command line, or those that want to be split into separate po) and extract messages
# NOTE: toplevel_sources may grow, dynamically, but only at the end.
i = 0
while i < len (toplevel_sources):
  xmldoc = parseFile (toplevel_sources[i])
  po_loc = os.path.join (os.path.dirname (toplevel_sources[i]), "po")
  po_id = xmldoc.documentElement.getAttribute ("po_id")
  if (po_id == ""):
    po_id = default_po
  elif (xmldoc.documentElement.hasAttribute ("po_path")):
    po_loc = os.path.join (os.path.dirname (toplevel_sources[i]), xmldoc.documentElement.getAttribute ("po_path"))
  if (po_id == ""):
    sys.stderr.write ("WARNING: No po_id attribute on file " + toplevel_sources[i] + " and no default specified. Skipping.\n")
    continue
  initialize_pot_file (po_id, po_loc)
  handleSubFile (toplevel_sources[i])  # Some duplication of parsing, instead of duplication of code
  i += 1

#######
# Run xgettext on all generated .pot.cpp files, and - unless --extract-only - merge, compile, install
for po_id in initialized_pot_files:
  p_outdir = outdir
  if (p_outdir == ""):
    p_outdir = po_file_install_locations[po_id]
  potcppfile = os.path.join (p_outdir, po_id + ".pot.cpp")
  templatename = "rkward__" + po_id
  finalpotfile = os.path.join (p_outdir, templatename + ".pot")
  # NOTE: using --no-location, as that just adds meaningless references to the temporary .pot.cpp-file.
  res = subprocess.call (XGETTEXT_CALL.split () + ["--no-location", "-o", finalpotfile, potcppfile])
  if (res):
    sys.stderr.write ("calling xgettext failed with exit code " + str (res))
  os.remove (potcppfile)
  if (not do_merge_install):
    continue
  # merge existing translations
  transfiles = os.listdir (os.path.join (p_outdir))
  for trans in transfiles:
    abstrans = os.path.join (p_outdir, trans)
    # is it really a translation file?
    if (trans.startswith (templatename + ".") and trans.endswith (".po") and ((len (templatename) + 6) >= len (trans) <= (len (templatename) + 7))):
      lang = trans.split ('.')[-2]
      res = subprocess.call (MSGMERGE.split () + ["-o", abstrans + ".new", abstrans, finalpotfile])
      if (res):
        sys.stderr.write ("Failed to merge messages for " + abstrans)
      else:
        os.remove (abstrans)
        os.rename (abstrans + ".new", abstrans)
      m_outdir = os.path.join (po_file_install_locations[po_id], lang, "LC_MESSAGES")
      if (not os.path.exists (m_outdir)):
        os.makedirs (m_outdir, 0755)
      res = subprocess.call (MSGFMT.split () + [abstrans, "-o", os.path.join (m_outdir, templatename + ".mo")])
      if (res):
        sys.stderr.write ("calling msgfmt on " + abstrans + " failed with exit code " + str (res))
