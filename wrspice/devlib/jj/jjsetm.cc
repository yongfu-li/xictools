
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
 $Id: jjsetm.cc,v 1.3 2016/10/16 01:33:10 stevew Exp $
 *========================================================================*/

/***************************************************************************
JSPICE3 adaptation of Spice3e2 - Copyright (c) Stephen R. Whiteley 1992
Author: 1992 Stephen R. Whiteley
****************************************************************************/

#include "jjdefs.h"


int
JJdev::setModl(int param, IFdata *data, sGENmodel *genmod)
{
    sJJmodel *model = static_cast<sJJmodel*>(genmod); 
    IFvalue *value = &data->v;

    switch (param) {
    case JJ_MOD_PI:
        model->JJpi = (value->iValue != 0);
        model->JJpiGiven = true;
        break;
    case JJ_MOD_RT:
        model->JJrtype = value->iValue;
        model->JJrtypeGiven = true;
        break;
    case JJ_MOD_IC:
        model->JJictype = value->iValue;
        model->JJictypeGiven = true;
        break;
    case JJ_MOD_VG:
        model->JJvg = value->rValue;
        model->JJvgGiven = true;
        break;
    case JJ_MOD_DV:
        model->JJdelv = value->rValue;
        model->JJdelvGiven = true;
        break;
    case JJ_MOD_CRT:
        model->JJcriti = value->rValue;
        model->JJcritiGiven = true;
        break;
    case JJ_MOD_CAP:
        model->JJcap = value->rValue;
        model->JJcapGiven = true;
        break;
    case JJ_MOD_R0:
        model->JJr0 = value->rValue;
        model->JJr0Given = true;
        break;
    case JJ_MOD_RN:
        model->JJrn = value->rValue;
        model->JJrnGiven = true;
        break;
    case JJ_MOD_CCS:
        model->JJccsens = value->rValue;
        model->JJccsensGiven = true;
        break;
    case JJ_MOD_ICF:
        model->JJicFactor = value->rValue;
        model->JJicfGiven = true;
        break;
    case JJ_MOD_VSHUNT:
        model->JJvShunt = value->rValue;
        model->JJvShuntGiven = true;
        break;
    case JJ_MOD_JJ:
        break;
    default:
        return (E_BADPARM);
    }
    return (OK);
}