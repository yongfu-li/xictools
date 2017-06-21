
/*========================================================================*
 *                                                                        *
 *  XICTOOLS Integrated Circuit Design System                             *
 *  Copyright (c) 1996 Whiteley Research Inc, all rights reserved.        *
 *                                                                        *
 *                                                                        *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,      *
 *   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES      *
 *   OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-        *
 *   INFRINGEMENT.  IN NO EVENT SHALL STEPHEN R. WHITELEY BE LIABLE       *
 *   FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION      *
 *   OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN           *
 *   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN         *
 *   THE SOFTWARE.                                                        *
 *                                                                        *
 *========================================================================*
 *                                                                        *
 * Device Library                                                         *
 *                                                                        *
 *========================================================================*
 $Id: vbicic.cc,v 1.1 2008/07/04 23:14:44 stevew Exp $
 *========================================================================*/

/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
Model Author: 1995 Colin McAndrew Motorola
Spice3 Implementation: 2003 Dietmar Warning DAnalyse GmbH
**********/

#include "vbicdefs.h"

#define VBICnextModel      next()
#define VBICnextInstance   next()
#define VBICinstances      inst()


// This routine gets the device initial conditions for the VBICs
// from the RHS vector
//
int
VBICdev::getic(sGENmodel *genmod, sCKT *ckt)
{
    sVBICmodel *model = static_cast<sVBICmodel*>(genmod);
    sVBICinstance *here;

    /*
     * grab initial conditions out of rhs array.   User specified, so use
     * external nodes to get values
     */

    for( ; model ; model = model->VBICnextModel) {
        for(here = model->VBICinstances; here ; here = here->VBICnextInstance) {

//            if (here->VBICowner != ARCHme) continue;

            if(!here->VBICicVBEGiven) {
                here->VBICicVBE = 
                        *(ckt->CKTrhs + here->VBICbaseNode) - 
                        *(ckt->CKTrhs + here->VBICemitNode);
            }
            if(!here->VBICicVCEGiven) {
                here->VBICicVCE = 
                        *(ckt->CKTrhs + here->VBICcollNode) - 
                        *(ckt->CKTrhs + here->VBICemitNode);
            }
        }
    }
    return(OK);
}