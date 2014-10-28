/***************************************************************************
                          xmlhelper.h  -  description
                             -------------------
    begin                : Fri May 6 2005
    copyright            : (C) 2005, 2007, 2011, 2014 by Thomas Friedrichsmeier
    email                : tfry@users.sourceforge.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef XMLHELPER_H
#define XMLHELPER_H

#include <qdom.h>

class RKMessageCatalog;

/** a helper type used to pass a list of direct child elements of a node */
typedef QList<QDomElement> XMLChildList;

/** This class contains some convenience functions for parsing XML files (DOM). RKComponents should retrieve the instance appropriate for parsing their
XML elements from the parent component: @see RKComponent::xmlHelper() .

The functions in this class provide error-messages for illegal/problematic input. Warning/Error messages will always be printed using the standard debugging framework (shown according to user settings).

@author Thomas Friedrichsmeier
*/
class XMLHelper {
public:
/** create an instance of XMLHelper.
 @param filename the name of the file to parse. The file is not yet opened on construction. Use openXMLFile() for that. 
 @param default_catalog message catalog to use in case none is specified in the xml file itself. */
	XMLHelper (const QString &filename, const RKMessageCatalog *default_catalog=0);
/** destructor */
	~XMLHelper ();
/** Return the a pointer to the message catalog in use. This may - or may not - be the same as specified as default catalog in the constructor.
    Guaranteed to be non-null (but not guaranteed to be non-empty). */
	const RKMessageCatalog *messageCatalog () const { return catalog; };

/** Open the filename set in the constructor (read-only) and do basic parsing. Internally, the file will be closed right away, so there is no need to call an additional closeFile-equivalent. Once the returned element (and any copies you make of it) goes out of scope, the entire element-tree allocated will be freed,
but you can re-open the file, if needed.
@param debug_level level of debug message to generate if opening/parsing fails
@param with_includes should the helper take care of resolving "include" elements?
@param with_snippets should the helper take care of resolving "insert" elements?
@returns the document-element of the file. */
	QDomElement openXMLFile (int debug_level, bool with_includes=true, bool with_snippets=true);

/** resolve "snippet" elements in the given element. It is assumed that the from element will contain a "snippets" section, i.e. it is generally a document. */
	QDomElement resolveSnippets (QDomElement &from);

/** returns all (direct) child elements with a given tag-name of the given parent
@param parent the element whose children to return
@param name the tag-name to look for (if none given, will return all children)
@param debug_level level of debug message to generate in case of failure
@returns a list of child elements (you'll have to call toElement () on the list items), in the order of occurrence in the XML file */
	XMLChildList getChildElements (const QDomElement &parent, const QString &name, int debug_level);

/** like getChildElements, but tries to retrieve exactly one element. Throws an error, if no such element, or more than one such element was found.
@param parent the element whose children to search
@param name the tag-name to look for
@param debug_level level of debug message to generate in case of failure
@returns the element found */
	QDomElement getChildElement (const QDomElement &parent, const QString &name, int debug_level);

/** find the first child element of parent, that has a certain attribute
@param parent the element whose children to search
@param attribute_name the attribute name of the attribute to search for
@param attribute_value the attribute value of the attribute to search for. If this a null string, each element containing the attribute qualifies
@param recursive do a recursive search? If false, only direct children will be looked at
@param debug_level level of debug message to generate in case of failure
@returns the element found */
	QDomElement findElementWithAttribute (const QDomElement &parent, const QString &attribute_name, const QString &attribute_value, bool recursive, int debug_level);

/** like findElementWithAttribute, but returns all such elements
@param parent the element whose children to search
@param attribute_name the attribute name of the attribute to search for
@param attribute_value the attribute value of the attribute to search for. If this a null string, each element containing the attribute qualifies
@param recursive do a recursive search? If false, only direct children will be looked at
@param debug_level level of debug message to generate in case of failure
@returns the element found */
	XMLChildList findElementsWithAttribute (const QDomElement &parent, const QString &attribute_name, const QString &attribute_value, bool recursive, int debug_level);

/** returns the value of a string attribute (Note: most get...Attribute functions use this function internally)
@param element the element whose attributes to search
@param name the name of the attribute to read
@param def default value to return if no such attribute is given
@param debug_level level of debug message to generate in case of failure (i.e. no such attribute was found)
@returns the value of the given attribute or the given default */
	QString getStringAttribute (const QDomElement &element, const QString &name, const QString &def, int debug_level);
/** same as getStringAttribute(), but tries to translate the string, before returning it. Does not translate def! */
	QString i18nStringAttribute (const QDomElement &element, const QString &name, const QString &def, int debug_level);

/** checks whether the given attribute is one of the allowed string values and returns the number of the value in the list (or the default)
@param element the element whose attributes to search
@param name the name of the attribute to read
@param values a list of allowed values given as a QString separated by ';', e.g. "menu;entry" allows the values "menu" (returns 0) or "entry" (returns 1)
@param def default value to return if no such attribute is given or does not hold a legal value
@param debug_level level of debug message to generate in case of failure (i.e. no such attribute was found) Note that if the given attribute is found, but is not a valid value, an error-message will be shown regardless of this setting, but highestError () will still use debug_level)
@returns the index of the value of the given attribute or the given default (see parameter values) */
	int getMultiChoiceAttribute (const QDomElement &element, const QString &name, const QString &values, int def, int debug_level);

/** returns the value of an integer attribute
@param element the element whose attributes to search
@param name the name of the attribute to read
@param def default value to return if no such attribute is given
@param debug_level level of debug message to generate in case of failure (i.e. no such attribute was found, or attribute was not an integer. Note that if the given attribute is found, but is not a valid integer, an error-message will be shown regardless of this setting, but highestError () will still use debug_level)
@returns the value of the given attribute or the given default */
	int getIntAttribute (const QDomElement &element, const QString &name, int def, int debug_level);

/** returns the value of a numeric (double) attribute
@param element the element whose attributes to search
@param name the name of the attribute to read
@param def default value to return if no such attribute is given
@param debug_level level of debug message to generate in case of failure (i.e. no such attribute was found, or attribute was not an integer. Note that if the given attribute is found, but is not a valid integer, an error-message will be shown regardless of this setting, but highestError () will still use debug_level)
@returns the value of the given attribute or the given default */
	double getDoubleAttribute (const QDomElement &element, const QString &name, double def, int debug_level);

/** returns the value of a boolean attribute ("true" or "false")
@param element the element whose attributes to search
@param name the name of the attribute to read
@param def default value to return if no such attribute is given
@param debug_level level of debug message to generate in case of failure (i.e. no such attribute was found. Note that if the given attribute is found, but is not a valid boolean, an error-message will be shown regardless of this setting, but highestError () will still use debug_level)
@returns true or false based on the value of the given attribute or the given default */
	bool getBoolAttribute (const QDomElement &element, const QString &name, bool def, int debug_level);

/** Gets a string representation of whatever is *inside* the element. Contrary to QDomElement::text(), this includes child tags.
 * Text is normalized, i18n'ed, put inside '<p></p>'-tags (unless empty), and double newlines are split into separate paragraphs.
 * @param element the element of interest
 * @param debug_level level of debug message to generate in case of failure (i.e. the element is null)
 * @returns the contents as a QString (may be empty) */
	QString i18nElementText (const QDomElement &element, int debug_level);

/** displays a custom-error message (also used internally by XMLHelper to display errors
@param in_node a pointer to the node/element to which the error relates (or 0). If given and non-zero, a "backtrace" of where the error is located will be generated
@param message the error-message to display
@param debug_level the debug level to show the message at (highestError () will be adujsted if applicable)
@param message_level sometime you may want to make sure your message is being shown even if it is not very important to your code. For instance, if there is a typo/illegal value in an optional setting, your code can continue using a reasonable default, but the user should still be notified of this error. If you omit this parameter or set it to something smaller that debug_level, debug_level will be used instead. */
	void displayError (const QDomNode *in_node, const QString &message, int debug_level, int message_level=-1);
private:
/** copy the node list into a child list. The main effect is that a child list is not updated according to document changes */
	XMLChildList nodeListToChildList (const QDomNodeList &from);
	void replaceWithChildren (QDomNode *replaced, const QDomElement &replacement_parent);
	QString filename;
	const RKMessageCatalog *catalog;
};

#endif
