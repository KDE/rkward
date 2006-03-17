/***************************************************************************
                          xmlhelper.h  -  description
                             -------------------
    begin                : Fri May 6 2005
    copyright            : (C) 2005 by Thomas Friedrichsmeier
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
#include <qvaluelist.h>

/** a helper type used to pass a list of direct child elements of a node */
typedef QValueList<QDomElement> XMLChildList;

/** This class contains some convenience functions for parsing XML files (DOM). Usually you will use a static instance of this class (getStaticHelper ()), which will be created early in rkward initialization. The error-logs will be reset every time you open a new XML-file using openXMLFile (). This is fine as long as you are parsing files one by one instead of mixing several files. In the latter case you will want to create additional instances of XMLHelper (it's quite possible, this mechanism will be changed, but I want to get going before considering all implications ;-)).

The functions in this class provide error-messages for illegal/problematic input. Information on the error status of the last commands is provided. More documentation to come once the API is somewhat finalized.

Warning/Error messages will always be printed using the standard debugging framework (shown according to user settings).

TODO: Probably it's not really clever to use the debugging-framework for showing error-messages in XML-file parsing. Anyway, the only function to adjust in order to change this would be displayError ().

TODO: Either something like push-context/pop-context or an added closeFile (). This way, when parsing several XML-files, we'd still get the correct messages. Internally a stack of filenames and highest_errors would be kept.

@author Thomas Friedrichsmeier
*/
class XMLHelper {
public:
/** create an instance of XMLHelper. Usually you will use the instance returned by getStaticHelper () instead of creating a new instance. */
	XMLHelper ();
/** destrcutor */
	~XMLHelper ();
	
/** open the given filename (read-only) and do basic parsing. Internally, the file will be closed right away, so there is no need to call an additional closeFile-equivalent. Once the returned element (and any copies you make of it) goes out of scope, the entire element-tree allocated will be freed.
When calling this function, highestError () will be reset to 0.
@param filename the name of the file to parse
@param debug_level level of debug message to generate if opening/parsing fails
@returns the document-element of the file. */
	QDomElement openXMLFile (const QString &filename, int debug_level);

/** returns all (direct) child elements with a given tag-name of the given parent
@param parent the element whose children to return
@param name the tag-name to look for (if none given, will return all children)
@param debug_level level of debug message to generate in case of failure
@returns a list of child elements (you'll have to call toElement () on the list items), in the order of occurence in the XML file */
	XMLChildList getChildElements (const QDomElement &parent, const QString &name, int debug_level);

/** like getChildElements, but tries to retrieve exactly one element. Throws an error, if no such element, or more than one such element was found.
@param parent the element whose children to search
@param name the tag-name to look for
@param debug_level level of debug message to generate in case of failure
@returns the element found */
	QDomElement getChildElement (const QDomElement &parent, const QString &name, int debug_level);

/** returns the value of a string attribute (Note: most get...Attribute functions use this function internally)
@param element the element whose attributes to search
@param name the name of the attribute to read
@param def default value to return if no such attribute is given
@param debug_level level of debug message to generate in case of failure (i.e. no such attribute was found)
@returns the value of the given attribute or the given default */
	QString getStringAttribute (const QDomElement &element, const QString &name, const QString &def, int debug_level);

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

/** @returns the level of the most severe error since the last call to openXMLFile () (based on the debug_level options passed to XMLHelper () */
	int highestError () { return (highest_error); };

/** displays a custom-error message (also used internally by XMLHelper to display errors
@param in_node a pointer to the node/element to which the error relates (or 0). If given and non-zero, a "backtrace" of where the error is located will be generated
@param message the error-message to display
@param debug_level the debug level to show the message at (highestError () will be adujsted if applicable)
@param message_level sometime you may want to make sure your message is being shown even if it is not very important to your code. For instance, if there is a typo/illegal value in an optional setting, your code can continue using a reasonable default, but the user should still be notified of this error. If you omit this parameter or set it to something smaller that debug_level, debug_level will be used instead. */
	void displayError (const QDomNode *in_node, const QString &message, int debug_level, int message_level=-1);
	
/** @returns a pointer to the default instance of XMLHelper (if none has been created so far, this will happen automatically. */
	static XMLHelper *getStaticHelper ();
private:
	int highest_error;
	static XMLHelper *static_xml_helper;
	QString filename;
};

#endif
