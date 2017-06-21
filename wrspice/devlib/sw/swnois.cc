
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
 $Id: swnois.cc,v 1.5 2015/08/08 01:51:12 stevew Exp $
 *========================================================================*/

/***************************************************************************
JSPICE3 adaptation of Spice3e2 - Copyright (c) Stephen R. Whiteley 1992
Copyright 1990 Regents of the University of California.  All rights reserved.
Authors: 1987 Gary W. Ng
         1992 Stephen R. Whiteley
****************************************************************************/

#include <stdio.h>
#include "swdefs.h"
#include "noisdefs.h"


//    This routine names and evaluates all of the noise sources
//    associated with voltage- controlled switches.  It starts with the
//    model *firstModel and traverses all of its instances.  It then
//    proceeds to any other models on the linked list.  The total output
//    noise density generated by the SW's is summed in the variable
//    "OnDens".
//
int
SWdev::noise(int mode, int operation, sGENmodel *genmod, sCKT *ckt,
    sNdata *data, double *OnDens)
{
    char swname[N_MXVLNTH];
    sSWmodel *model = static_cast<sSWmodel*>(genmod);
    if (operation == N_OPEN) {
        // see if we have to to produce a summary report
        // if so, name the noise generator
        if (static_cast<sNOISEAN*>(ckt->CKTcurJob)->NStpsSm == 0)
            return (OK);
        if (mode == N_DENS) {
            for ( ; model; model = model->next()) {
                sSWinstance *inst;
                for (inst = model->inst(); inst; inst = inst->next()) {

                    (void)sprintf(swname, "onoise_%s", (char*)inst->GENname);
                    Realloc(&data->namelist, data->numPlots+1,
                        data->numPlots);
                    ckt->newUid(&data->namelist[data->numPlots++],
                        0, swname, UID_OTHER);
                    // we've added one more plot
                }
            }
            return (OK);
        }

        if (mode == INT_NOIZ) {
            for ( ; model; model = model->next()) {
                sSWinstance *inst;
                for (inst = model->inst(); inst; inst = inst->next()) {

                    (void)sprintf(swname, "onoise_total_%s",
                        (char*)inst->GENname);
                    Realloc(&data->namelist, data->numPlots+2,
                        data->numPlots);
                    ckt->newUid(&data->namelist[data->numPlots++],
                        0, swname, UID_OTHER);
                    (void)sprintf(swname, "inoise_total_%s",
                        (char*)inst->GENname);
                    ckt->newUid(&data->namelist[data->numPlots++],
                        0, swname, UID_OTHER);
                    // we've added two more plots
                }
            }
        }
        return (OK);
    }

    if (operation == N_CALC) {
        if (mode == N_DENS) {
            for ( ; model; model = model->next()) {
                sSWinstance *inst;
                for (inst = model->inst(); inst; inst = inst->next()) {

                    int current_state =
                        (int)*(ckt->CKTstate0 + inst->GENstate);
                    double noizDens;
                    double lnNdens;
                    NevalSrc(&noizDens, &lnNdens, ckt, THERMNOISE,
                        inst->SWposNode, inst->SWnegNode,
                        current_state ?
                        model->SWonConduct : model->SWoffConduct);

                    *OnDens += noizDens;

                    if (data->delFreq == 0.0) { 

                        // if we haven't done any previous
                        // integration, we need to initialize our
                        // "history" variables

                        inst->SWnVar[LNLSTDENS] = lnNdens;

                        // clear out our integration variable if
                        // it's the first pass

                        if (data->freq ==
                            ((sNOISEAN*)ckt->CKTcurJob)->JOBac.fstart()) {
                            inst->SWnVar[OUTNOIZ] = 0.0;
                        }
                    }
                    else {
                        // data->delFreq != 0.0
                        // (we have to integrate)
                        //
                        double tempOutNoise;
                        double tempInNoise;
                        tempOutNoise = data->integrate(noizDens, lnNdens,
                            inst->SWnVar[LNLSTDENS]);
                        tempInNoise = data->integrate(noizDens*data->GainSqInv,
                            lnNdens + data->lnGainInv,
                            inst->SWnVar[LNLSTDENS] + data->lnGainInv);
                        inst->SWnVar[OUTNOIZ] += tempOutNoise;
                        inst->SWnVar[INNOIZ] += tempInNoise;
                        data->outNoiz += tempOutNoise;
                        data->inNoise += tempInNoise;
                        inst->SWnVar[LNLSTDENS] = lnNdens;
                    }
                    if (data->prtSummary)
                        data->outpVector[data->outNumber++] = noizDens;
                }
            }
            return (OK);
        }

        if (mode == INT_NOIZ) {
            // already calculated, just output
            if (static_cast<sNOISEAN*>(ckt->CKTcurJob)->NStpsSm == 0)
                return (OK);
            for ( ; model; model = model->next()) {
                sSWinstance *inst;
                for (inst = model->inst(); inst; inst = inst->next()) {

                    data->outpVector[data->outNumber++] =
                        inst->SWnVar[OUTNOIZ];
                    data->outpVector[data->outNumber++] =
                        inst->SWnVar[INNOIZ];
                }
            }
        }
    }

    return (OK);
}