/***************************************************************************
                          rkgraphicsdevice_setup  -  description
                             -------------------
    begin                : Mon Mar 18 20:06:08 CET 2013
    copyright            : (C) 2013 by Thomas Friedrichsmeier 
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

/******************************* ACKNOWLEDGEMENT ***************************
 * 
 * Much of the code in this file is based on, or even copied from package qtutils, version 0.1-3
 * by Deepayan Sarkar. Package qtutils is available from http://qtinterfaces.r-forge.r-project.org
 * under GNU LPGL 2 or later.
 * 
 ***************************************************************************/

#include "../rkrsupport.h"

#ifdef TRUE
#	undef TRUE
#endif
#ifdef FALSE
#	undef FALSE
#endif
#define R_USE_PROTOTPYES 1

extern "C" {
#include <R_ext/GraphicsEngine.h>
#include <Rversion.h>
}

// rcolor typedef added in R 3.0.0
#ifndef rcolor
#define rcolor unsigned int
#endif

struct RKGraphicsDeviceDesc {
	bool init (pDevDesc dev, double pointsize, const QStringList &family, rcolor bg);
	int devnum;
	double width, height;
	int dpix, dpiy;
	QString getFontFamily (bool symbolfont) const {
		if (symbolfont) return default_symbol_family;
		return default_family;
	}
	QString default_family;
	QString default_symbol_family;
	pDevDesc rdevdesc;
};

#include "rkgraphicsdevice_stubs.cpp"

// No, I do not really understand what this is for.
// Mostly trying to mimick the X11 device's behavior, here.
#define RKGD_DPI 72.0

void RKStartGraphicsDevice (double width, double height, double pointsize, const QStringList &family, rcolor bg, const char* title, bool antialias) {
	if (width <= 0 || height <= 0) {
		Rf_error ("Invalid width or height: (%g, %g)", width, height);
	}
	RKGraphicsDeviceDesc *desc = new RKGraphicsDeviceDesc;
	desc->width = width;
	desc->height = height;

	R_GE_checkVersionOrDie (R_GE_version);
	R_CheckDeviceAvailable ();
	pDevDesc dev;
	BEGIN_SUSPEND_INTERRUPTS {
		/* Allocate and initialize the device driver data */
		dev = (pDevDesc) calloc (1, sizeof(DevDesc));
		// NOTE: The call to RKGraphicsDeviceBackendTransmitter::instance(), here is important beyond error checking. It might *create* the instance and connection, if this is the first use.
		if (!(dev && RKGraphicsDeviceBackendTransmitter::instance () && desc->init (dev, pointsize, family, bg))) {
			free (dev);
			delete (desc);
			desc = 0;
		} else {
			desc->devnum = 0;	// graphics engine will send an Activate-event, before we were even
								// able to see our own devnum and call RKD_Create. Therefore, intialize
								// devnum to 0, so as not to confuse the frontend
			pGEDevDesc gdd = GEcreateDevDesc (dev);
			gdd->displayList = R_NilValue;
			GEaddDevice2 (gdd, "RKGraphicsDevice");
		}
	} END_SUSPEND_INTERRUPTS;

	if (desc) {
		desc->devnum = curDevice ();
		RKD_Create (desc->width, desc->height, dev, title, antialias);
	} else {
		Rf_error("unable to start device");
	}
}

SEXP RKStartGraphicsDevice (SEXP width, SEXP height, SEXP pointsize, SEXP family, SEXP bg, SEXP title, SEXP antialias) {
	RKStartGraphicsDevice (Rf_asReal (width), Rf_asReal (height), Rf_asReal (pointsize), RKRSupport::SEXPToStringList (family), R_GE_str2col (CHAR(Rf_asChar(bg))), CHAR(Rf_asChar(title)), Rf_asLogical (antialias));
	return R_NilValue;
}

bool RKGraphicsDeviceDesc::init (pDevDesc dev, double pointsize, const QStringList &family, rcolor bg) {
	default_family = family.value (0, "Helvetica");
	default_symbol_family = family.value (0, "Symbol");
	RKD_QueryResolution (&dpix, &dpiy);
	if (dpix <= 1) dpix = RKGD_DPI;
	if (dpiy <= 1) dpiy = RKGD_DPI;
	width *= dpix;
	height *= dpiy;
//	Rprintf ("dpi: %d * %d, dims: %f * %f\n", dpix, dpiy, width, height);

	dev->deviceSpecific = (void *) this;

	// pointsize?

	/*
	* Initial graphical settings
	*/
	dev->startfont = 1;
	dev->startps = pointsize;
	dev->startcol = R_RGB(0, 0, 0);
	dev->startfill = bg;
	dev->startlty = LTY_SOLID;
	dev->startgamma = 1;
	/*
	* Device physical characteristics
	*/
	dev->left   = dev->clipLeft   = 0;
	dev->right  = dev->clipRight  = width;
	dev->bottom = dev->clipBottom = height;
	dev->top    = dev->clipTop    = 0;
	dev->cra[0] = 0.9 * pointsize * (dpix / RKGD_DPI);
	dev->cra[1] = 1.2 * pointsize * (dpiy / RKGD_DPI);
	dev->xCharOffset = 0.4900;
	dev->yCharOffset = 0.3333;
	dev->yLineBias = 0.2;
	dev->ipr[0] = 1.0 / dpix;
	dev->ipr[1] = 1.0 / dpiy;
	/*
	* Device capabilities
	*/
	dev->canClip = TRUE;
	dev->canHAdj = 2;
	dev->canChangeGamma = FALSE;
	dev->displayListOn = TRUE;

	dev->hasTextUTF8 = TRUE;
	dev->textUTF8 = RKD_TextUTF8;
	dev->strWidthUTF8 = RKD_StrWidthUTF8;
	dev->wantSymbolUTF8 = TRUE;
	dev->useRotatedTextInContour = TRUE;

#if R_VERSION >= R_Version (2, 14, 0)
	dev->haveTransparency = 2;
	dev->haveTransparentBg = 2; // FIXME. Do we really? Check.
	dev->haveRaster = 2;
	dev->haveCapture = 2;
	dev->haveLocator = 2;
#endif

#if R_VERSION >= R_Version (2, 12, 0)
	/*
	* Mouse events
	*/
	dev->canGenMouseDown = TRUE;
	dev->canGenMouseMove = TRUE;
	dev->canGenMouseUp = TRUE; 
	dev->canGenKeybd = TRUE;

	// gettingEvent; This is set while getGraphicsEvent is actively
	// looking for events
	dev->eventHelper = RKD_EventHelper;
	dev->onExit = RKD_onExit;
#endif

	/*
	* Device functions
	*/
	dev->activate = RKD_Activate;
	dev->circle = RKD_Circle;
	dev->clip = RKD_Clip;
	dev->close = RKD_Close;
	dev->deactivate = RKD_Deactivate;
	dev->locator = RKD_Locator;
	dev->line = RKD_Line;
	dev->metricInfo = RKD_MetricInfo;
	dev->mode = RKD_Mode;
	dev->newPage = RKD_NewPage;
	dev->polygon = RKD_Polygon;
	dev->polyline = RKD_Polyline;
	dev->rect = RKD_Rect;
	dev->size = RKD_Size;
	// dev->onexit = RKD_OnExit; Called on user interrupts. NULL is OK.
#if R_VERSION >= R_Version (2, 11, 0)
	dev->raster = RKD_Raster;
	dev->cap = RKD_Capture;
#endif
	dev->newFrameConfirm = RKD_NewFrameConfirm;

	return true;
}
 
