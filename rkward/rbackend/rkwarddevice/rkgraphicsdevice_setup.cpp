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
}

struct RKGraphicsDeviceDesc {
	bool initRDevDesc (pDevDesc dev, double pointsize);
	int devnum;
	double width, height;
	QString default_family;
	pDevDesc rdevdesc;
};

#include "rkgraphicsdevice_stubs.cpp"

#define RKGD_DPI 72

void RKStartGraphicsDevice (double width, double height, double pointsize, const QString &family) {
	if (width <= 0 || height <= 0) {
		Rf_error ("Invalid width or height: (%g, %g)", width, height);
	}
	RKGraphicsDeviceDesc *desc = new RKGraphicsDeviceDesc;
	desc->width = width * RKGD_DPI;
	desc->height = height * RKGD_DPI;
	desc->default_family = family;

	R_GE_checkVersionOrDie (R_GE_version);
	R_CheckDeviceAvailable ();
	pDevDesc dev;
	BEGIN_SUSPEND_INTERRUPTS {
		/* Allocate and initialize the device driver data */
		dev = (pDevDesc) calloc (1, sizeof(DevDesc));
		if (!(dev && desc->initRDevDesc (dev, pointsize) && RKGraphicsDeviceBackendTransmitter::instance ())) {
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
		RKD_Create (desc->width, desc->height, dev);
	} else {
		Rf_error("unable to start device");
	}
}

SEXP RKStartGraphicsDevice (SEXP width, SEXP height, SEXP pointsize, SEXP family
#warning TODO: add more params for compatibility with X11()
) {
	RKStartGraphicsDevice (Rf_asReal (width), Rf_asReal (height), Rf_asReal (pointsize), RKRSupport::SEXPToString (family));
	return R_NilValue;
}

bool RKGraphicsDeviceDesc::initRDevDesc (pDevDesc dev, double pointsize) {
	dev->deviceSpecific = (void *) this;

	// pointsize?

	/*
	* Initial graphical settings
	*/
	dev->startfont = 1;
	dev->startps = pointsize;
	dev->startcol = R_RGB(0, 0, 0);
	dev->startfill = R_TRANWHITE;
	dev->startlty = LTY_SOLID;
	dev->startgamma = 1;
	/*
	* Device physical characteristics
	*/
	dev->left   = dev->clipLeft   = 0;
	dev->right  = dev->clipRight  = width;
	dev->bottom = dev->clipBottom = height;
	dev->top    = dev->clipTop    = 0;
	dev->cra[0] = 0.9 * pointsize * 96/72;
	dev->cra[1] = 1.2 * pointsize * 96/72;
	dev->xCharOffset = 0.4900;
	dev->yCharOffset = 0.3333;
	dev->yLineBias = 0.1;
	dev->ipr[0] = 1.0 / RKGD_DPI;
	dev->ipr[1] = 1.0 / RKGD_DPI;
	/*
	* Device capabilities
	*/
	dev->canClip = FALSE; // FIXME. can clip, but then selection becomes weird
	dev->canHAdj = 2;
	dev->canChangeGamma = FALSE;
	dev->displayListOn = TRUE;

	dev->hasTextUTF8 = TRUE;
	dev->textUTF8 = RKD_TextUTF8;
	dev->strWidthUTF8 = RKD_StrWidthUTF8;
	dev->wantSymbolUTF8 = TRUE;
	dev->useRotatedTextInContour = TRUE;

	dev->haveTransparency = 2;
	dev->haveTransparentBg = 2; // FIXME. Do we really? Check.
	dev->haveRaster = 1;
	dev->haveCapture = 1;
	dev->haveLocator = 2;

	/*
	* Mouse events
	*/
//     dev->canGenMouseDown = TRUE;
//     dev->canGenMouseMove = TRUE;
//     dev->canGenMouseUp = TRUE; 
//     dev->canGenKeybd = TRUE;

	// gettingEvent; This is set while getGraphicsEvent is actively
	// looking for events

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
	dev->size = NULL; // RKD_Size;
	// dev->onexit = RKD_OnExit; NULL is OK
	// dev->getEvent = SEXP (*getEvent)(SEXP, const char *);
	dev->newFrameConfirm = RKD_NewFrameConfirm;

	return true;
}
 
