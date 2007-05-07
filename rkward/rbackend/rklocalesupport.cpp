/***************************************************************************
                          rklocalesupport  -  description
                             -------------------
    begin                : Sun Mar 11 2007
    copyright            : (C) 2007 by Thomas Friedrichsmeier
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

#include "rklocalesupport.h"

// see https://sourceforge.net/tracker/?func=detail&atid=459009&aid=1698809&group_id=50231
#ifdef __FreeBSD__
# include <langinfo.h>
#endif

#include <qtextcodec.h>

/* NOTE: The code in this file is an almost literal copy taken from setupLocaleMapper in qtextcodec.cpp in Qt 3.3.8 !*/

QTextCodec *checkForCodec(const char *name) {
    QTextCodec *c = QTextCodec::codecForName(name);
    if (!c) {
        const char *at = strchr(name, '@');
        if (at) {
            QCString n(name, at - name + 1);
            c = QTextCodec::codecForName(n.data());
        }
    }
    return c;
}

/* locale names mostly copied from XFree86 */
static const char * const iso8859_2locales[] = {
    "croatian", "cs", "cs_CS", "cs_CZ","cz", "cz_CZ", "czech", "hr",
    "hr_HR", "hu", "hu_HU", "hungarian", "pl", "pl_PL", "polish", "ro",
    "ro_RO", "rumanian", "serbocroatian", "sh", "sh_SP", "sh_YU", "sk",
    "sk_SK", "sl", "sl_CS", "sl_SI", "slovak", "slovene", "sr_SP", 0 };

static const char * const iso8859_3locales[] = {
    "eo", 0 };

static const char * const iso8859_4locales[] = {
    "ee", "ee_EE", 0 };

static const char * const iso8859_5locales[] = {
    "mk", "mk_MK", "sp", "sp_YU", 0 };

static const char * const cp_1251locales[] = {
    "be", "be_BY", "bg", "bg_BG", "bulgarian", 0 };

static const char * const pt_154locales[] = {
    "ba_RU", "ky", "ky_KG", "kk", "kk_KZ", 0 };

static const char * const iso8859_6locales[] = {
    "ar_AA", "ar_SA", "arabic", 0 };

static const char * const iso8859_7locales[] = {
    "el", "el_GR", "greek", 0 };

static const char * const iso8859_8locales[] = {
    "hebrew", "he", "he_IL", "iw", "iw_IL", 0 };

static const char * const iso8859_9locales[] = {
    "tr", "tr_TR", "turkish", 0 };

static const char * const iso8859_13locales[] = {
    "lt", "lt_LT", "lv", "lv_LV", 0 };

static const char * const iso8859_15locales[] = {
    "et", "et_EE",
    // Euro countries
    "br_FR", "ca_ES", "de", "de_AT", "de_BE", "de_DE", "de_LU", "en_IE",
    "es", "es_ES", "eu_ES", "fi", "fi_FI", "finnish", "fr", "fr_FR",
    "fr_BE", "fr_LU", "french", "ga_IE", "gl_ES", "it", "it_IT", "oc_FR",
    "nl", "nl_BE", "nl_NL", "pt", "pt_PT", "sv_FI", "wa_BE",
    0 };

static const char * const koi8_ulocales[] = {
    "uk", "uk_UA", "ru_UA", "ukrainian", 0 };

static const char * const tis_620locales[] = {
    "th", "th_TH", "thai", 0 };

static const char * const tcvnlocales[] = {
    "vi", "vi_VN", 0 };

static bool try_locale_list( const char * const locale[], const char * lang )
{
    int i;
    for( i=0; locale[i] && *locale[i] && strcmp(locale[i], lang); i++ )
        ;
    return locale[i] != 0;
}

// For the probably_koi8_locales we have to look. the standard says
// these are 8859-5, but almost all Russian users use KOI8-R and
// incorrectly set $LANG to ru_RU. We'll check tolower() to see what
// tolower() thinks ru_RU means.

// If you read the history, it seems that many Russians blame ISO and
// Perestroika for the confusion.
//
// The real bug is that some programs break if the user specifies
// ru_RU.KOI8-R.

static const char * const probably_koi8_rlocales[] = {
    "ru", "ru_SU", "ru_RU", "russian", 0 };

static QTextCodec * ru_RU_hack( const char * i ) {
    QTextCodec * ru_RU_codec = 0;

    QCString origlocale = setlocale( LC_CTYPE, i );
    // unicode   koi8r   latin5   name
    // 0x044E    0xC0    0xEE     CYRILLIC SMALL LETTER YU
    // 0x042E    0xE0    0xCE     CYRILLIC CAPITAL LETTER YU
    int latin5 = tolower( 0xCE );
    int koi8r = tolower( 0xE0 );
    if ( koi8r == 0xC0 && latin5 != 0xEE ) {
        ru_RU_codec = QTextCodec::codecForName( "KOI8-R" );
    } else if ( koi8r != 0xC0 && latin5 == 0xEE ) {
        ru_RU_codec = QTextCodec::codecForName( "ISO 8859-5" );
    } else {
        // something else again... let's assume... *throws dice*
        ru_RU_codec = QTextCodec::codecForName( "KOI8-R" );
        qWarning( "QTextCodec: using KOI8-R, probe failed (%02x %02x %s)",
                  koi8r, latin5, i );
    }
    setlocale( LC_CTYPE, origlocale.data() );

    return ru_RU_codec;
}

QTextCodec *RKGetCurrentLocaleCodec () {

	QTextCodec *localeMapper = 0;

#ifdef Q_OS_WIN32
    localeMapper = QTextCodec::codecForName( "System" );
#else

#if defined (_XOPEN_UNIX) && !defined(Q_OS_QNX6) && !defined(Q_OS_OSF) && !defined(Q_OS_MAC)
    char *charset = nl_langinfo (CODESET);
    if ( charset )
        localeMapper = QTextCodec::codecForName( charset );
#endif

    if ( !localeMapper ) {
        // Very poorly defined and followed standards causes lots of code
        // to try to get all the cases...

        // Try to determine locale codeset from locale name assigned to
        // LC_CTYPE category.

        // First part is getting that locale name.  First try setlocale() which
        // definitely knows it, but since we cannot fully trust it, get ready
        // to fall back to environment variables.
        char * ctype = qstrdup( setlocale( LC_CTYPE, 0 ) );

        // Get the first nonempty value from $LC_ALL, $LC_CTYPE, and $LANG
        // environment variables.
        char * lang = qstrdup( getenv("LC_ALL") );
        if ( !lang || lang[0] == 0 || strcmp( lang, "C" ) == 0 ) {
            if ( lang ) delete [] lang;
            lang = qstrdup( getenv("LC_CTYPE") );
        }
        if ( !lang || lang[0] == 0 || strcmp( lang, "C" ) == 0 ) {
            if ( lang ) delete [] lang;
            lang = qstrdup( getenv("LANG") );
        }

        // Now try these in order:
        // 1. CODESET from ctype if it contains a .CODESET part (e.g. en_US.ISO8859-15)
        // 2. CODESET from lang if it contains a .CODESET part
        // 3. ctype (maybe the locale is named "ISO-8859-1" or something)
        // 4. locale (ditto)
        // 5. check for "@euro"
        // 6. guess locale from ctype unless ctype is "C"
        // 7. guess locale from lang

        // 1. CODESET from ctype if it contains a .CODESET part (e.g. en_US.ISO8859-15)
        char * codeset = ctype ? strchr( ctype, '.' ) : 0;
        if ( codeset && *codeset == '.' )
            localeMapper = checkForCodec( codeset + 1 );

        // 2. CODESET from lang if it contains a .CODESET part
        codeset = lang ? strchr( lang, '.' ) : 0;
        if ( !localeMapper && codeset && *codeset == '.' ) 
            localeMapper = checkForCodec( codeset + 1 );

        // 3. ctype (maybe the locale is named "ISO-8859-1" or something)
        if ( !localeMapper && ctype && *ctype != 0 && strcmp (ctype, "C") != 0 )
            localeMapper = checkForCodec( ctype );

        // 4. locale (ditto)
        if ( !localeMapper && lang && *lang != 0 )
            localeMapper = checkForCodec( lang );

        // 5. "@euro"
        if ( !localeMapper && ctype && strstr( ctype, "@euro" ) || lang && strstr( lang, "@euro" ) )
            localeMapper = QTextCodec::codecForName( "ISO 8859-15" );

        // 6. guess locale from ctype unless ctype is "C"
        // 7. guess locale from lang
        char * try_by_name = ctype;
        if ( ctype && *ctype != 0 && strcmp (ctype, "C") != 0 )
            try_by_name = lang;

        // Now do the guessing.
        if ( lang && *lang && !localeMapper && try_by_name && *try_by_name ) {
            if ( try_locale_list( iso8859_15locales, lang ) )
                localeMapper = QTextCodec::codecForName( "ISO 8859-15" );
            else if ( try_locale_list( iso8859_2locales, lang ) )
                localeMapper = QTextCodec::codecForName( "ISO 8859-2" );
            else if ( try_locale_list( iso8859_3locales, lang ) )
                localeMapper = QTextCodec::codecForName( "ISO 8859-3" );
            else if ( try_locale_list( iso8859_4locales, lang ) )
                localeMapper = QTextCodec::codecForName( "ISO 8859-4" );
            else if ( try_locale_list( iso8859_5locales, lang ) )
                localeMapper = QTextCodec::codecForName( "ISO 8859-5" );
            else if ( try_locale_list( iso8859_6locales, lang ) )
                localeMapper = QTextCodec::codecForName( "ISO 8859-6" );
            else if ( try_locale_list( iso8859_7locales, lang ) )
                localeMapper = QTextCodec::codecForName( "ISO 8859-7" );
            else if ( try_locale_list( iso8859_8locales, lang ) )
                localeMapper = QTextCodec::codecForName( "ISO 8859-8-I" );
            else if ( try_locale_list( iso8859_9locales, lang ) )
                localeMapper = QTextCodec::codecForName( "ISO 8859-9" );
            else if ( try_locale_list( iso8859_13locales, lang ) )
                localeMapper = QTextCodec::codecForName( "ISO 8859-13" );
            else if ( try_locale_list( tis_620locales, lang ) )
                localeMapper = QTextCodec::codecForName( "ISO 8859-11" );
            else if ( try_locale_list( koi8_ulocales, lang ) )
                localeMapper = QTextCodec::codecForName( "KOI8-U" );
            else if ( try_locale_list( cp_1251locales, lang ) )
                localeMapper = QTextCodec::codecForName( "CP 1251" );
            else if ( try_locale_list( pt_154locales, lang ) )
                localeMapper = QTextCodec::codecForName( "PT 154" );
            else if ( try_locale_list( probably_koi8_rlocales, lang ) )
                localeMapper = ru_RU_hack( lang );
        }

        delete [] ctype;
        delete [] lang;
    }
    if ( localeMapper && localeMapper->mibEnum() == 11 )
        localeMapper = QTextCodec::codecForName( "ISO 8859-8-I" );

    // If everything failed, we default to 8859-1
    // We could perhaps default to 8859-15.
    if ( !localeMapper )
        localeMapper = QTextCodec::codecForName( "ISO 8859-1" );
#endif
    return localeMapper;
}

