/***************************************************************************
                          rkgraphicsdevice  -  description
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

#include "rkgraphicsdevice_stubs.cpp"

struct RKGraphicsDeviceDesc {
	bool initRDevDesc (pDevDesc dev, double pointsize);
	int devnum;
	double width, height;
	QString default_family;
	pDevDesc rdevdesc;
};

#define RKGD_DPI 72

void RKStartGraphicsDevice (double width, double height, double pointsize, const QString &family) {
	if (width <= 0 || height <= 0) {
		Rf_error ("Invalid width or height: (%g, %g)", width, height);
	}
	RKGraphicsDeviceDesc *desc = new RKGraphicsDeviceDesc;
	desc->width = width * RKGD_DPI;
	desc->height = height * RKGD_DPI;
	desc->default_family = family;
//	_scene->setSceneRect(0.0, 0.0, width, height);

	R_GE_checkVersionOrDie (R_GE_version);
	R_CheckDeviceAvailable ();
	BEGIN_SUSPEND_INTERRUPTS {
		pDevDesc dev;
		/* Allocate and initialize the device driver data */
		if (!(dev = (pDevDesc) calloc (1, sizeof(DevDesc))))
			return 0; /* or error() */
		/* set up device driver or free 'dev' and error() */
		if (!desc->initRDevDesc (dev, pointsize, desc)) {
			free (dev);
			delete (desc);
			Rf_error("unable to start device");
		}
		pGEDevDesc gdd = GEcreateDevDesc (dev);
		gdd->displayList = R_NilValue;
		GEaddDevice2 (gdd, "QTScene");
	} END_SUSPEND_INTERRUPTS;

	desc->devnum = curDevice ();
	RKD_Create (width, height, desc);
}

SEXP RKStartGraphicsDevice (SEXP width, SEXP height, SEXP pointsize, SEXP family
#warning TODO: add more params for compatibility with X11
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
	dev->cra[0] = 0.9 * pointsize;
	dev->cra[1] = 1.2 * pointsize;
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
	dev->textUTF8 = (void (*)()) RKD_TextUTF8;
	dev->strWidthUTF8 = (double (*)()) RKD_StrWidthUTF8;
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
	dev->activate =    (void (*)()) RKD_Activate;
	dev->circle =      (void (*)()) RKD_Circle;
	dev->clip =        (void (*)()) RKD_Clip;
	dev->close =       (void (*)()) RKD_Close;
	dev->deactivate =  (void (*)()) RKD_Deactivate;
	dev->locator = (Rboolean (*)()) RKD_Locator;
	dev->line =        (void (*)()) RKD_Line;
	dev->metricInfo =  (void (*)()) RKD_MetricInfo;
	dev->mode =        (void (*)()) RKD_Mode;
	dev->newPage =     (void (*)()) RKD_NewPage;
	dev->polygon =     (void (*)()) RKD_Polygon;
	dev->polyline =    (void (*)()) RKD_Polyline;
	dev->rect =        (void (*)()) RKD_Rect;
	dev->size =        NULL; // (void (*)()) RKD_Size;
	// dev->onexit =      (void (*)()) RKD_OnExit; NULL is OK
	// dev->getEvent = SEXP (*getEvent)(SEXP, const char *);
	dev->newFrameConfirm = (Rboolean (*)()) RKD_NewFrameConfirm;

	return true;
}
 
