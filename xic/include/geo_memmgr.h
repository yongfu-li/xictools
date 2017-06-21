
/*========================================================================*
 *                                                                        *
 *  XICTOOLS Integrated Circuit Design System                             *
 *  Copyright (c) 2012 Whiteley Research Inc, all rights reserved.        *
 *                                                                        *
 *  WHITELEY RESEARCH INCORPORATED PROPRIETARY SOFTWARE                   *
 *                                                                        *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,      *
 *   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES      *
 *   OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-        *
 *   INFRINGEMENT.  IN NO EVENT SHALL WHITELEY RESEARCH INCORPORATED      *
 *   OR STEPHEN R. WHITELEY BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER     *
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,      *
 *   ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE       *
 *   USE OR OTHER DEALINGS IN THE SOFTWARE.                               *
 *                                                                        *
 *========================================================================*
 *                                                                        *
 * XIC Integrated Circuit Layout and Schematic Editor                     *
 *                                                                        *
 *========================================================================*
 $Id: geo_memmgr.h,v 5.7 2015/07/17 19:44:02 stevew Exp $
 *========================================================================*/

#ifndef GEO_MEMMGR_H
#define GEO_MEMMGR_H

#include "geo_box.h"
#include "geo_ylist.h"
#include "geo_zlist.h"
#include "geo_zdb.h"
#include "geo_rtree.h"
#include "memmgr_tmpl.h"


inline class cGEOmmgr *GEOmmgr();

class cGEOmmgr
{
public:
    friend inline cGEOmmgr *GEOmmgr() { return (cGEOmmgr::instancePtr); }

    cGEOmmgr();

    void stats(int*, int*);

    void collectTrash()
        {
            RTelem_db.collectTrash();
            Zlist_db.collectTrash();
            Ylist_db.collectTrash();
            Blist_db.collectTrash();
            BYlist_db.collectTrash();
            GEOblock_db.collectTrash();
        }

    void *new_RTelem()          { return (RTelem_db.newElem()); }
    void del_RTelem(RTelem *e)  { if (e) RTelem_db.delElem(e); }

    void *new_Zlist()           { return (Zlist_db.newElem()); }
    void del_Zlist(Zlist *e)    { if (e) Zlist_db.delElem(e); }

    void *new_Ylist()           { return (Ylist_db.newElem()); }
    void del_Ylist(Ylist *e)    { if (e) Ylist_db.delElem(e); }

    void *new_Blist()           { return (Blist_db.newElem()); }
    void del_Blist(Blist *e)    { if (e) Blist_db.delElem(e); }

    void *new_BYlist()          { return (BYlist_db.newElem()); }
    void del_BYlist(BYlist *e)  { if (e) BYlist_db.delElem(e); }

    void *new_GEOblock()        { return (GEOblock_db.newElem()); }
    void del_GEOblock(GEOblock *e) { if (e) GEOblock_db.delElem(e); }

private:
    MemMgr<RTelem> RTelem_db;
    MemMgr<Zlist> Zlist_db;
    MemMgr<Ylist> Ylist_db;
    MemMgr<Blist> Blist_db;
    MemMgr<BYlist> BYlist_db;
    MemMgr<GEOblock> GEOblock_db;

    static cGEOmmgr *instancePtr;
};

#endif
