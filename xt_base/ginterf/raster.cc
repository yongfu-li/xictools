
/*========================================================================*
 *                                                                        *
 *  XICTOOLS Integrated Circuit Design System                             *
 *  Copyright (c) 2005 Whiteley Research Inc, all rights reserved.        *
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
 * This library is available for non-commercial use under the terms of    *
 * the GNU Library General Public License as published by the Free        *
 * Software Foundation; either version 2 of the License, or (at your      *
 * option) any later version.                                             *
 *                                                                        *
 * A commercial license is available to those wishing to use this         *
 * library or a derivative work for commercial purposes.  Contact         *
 * Whiteley Research Inc., 456 Flora Vista Ave., Sunnyvale, CA 94086.     *
 * This provision will be enforced to the fullest extent possible under   *
 * law.                                                                   *
 *                                                                        *
 * This library is distributed in the hope that it will be useful,        *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 * Library General Public License for more details.                       *
 *                                                                        *
 * You should have received a copy of the GNU Library General Public      *
 * License along with this library; if not, write to the Free             *
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.     *
 *========================================================================*
 *                                                                        *
 * Ginterf Graphical Interface Library                                    *
 *                                                                        *
 *========================================================================*
 $Id: raster.cc,v 2.27 2012/03/17 22:45:30 stevew Exp $
 *========================================================================*/

// Routines for rendering on an in-core bitmap

#include <stdio.h>
#include <math.h>
#include "raster.h"
#include "grfont.h"
#include "polydecomp.h"


#ifndef M_PI
#define	M_PI  3.14159265358979323846  // pi
#endif

#define swap(a, b) {int t=a; a=b; b=t;}


// Draw pixel function.
//
void
RASparams::Pixel(int x, int y)
{
    if (y < 0 || y >= dev->height || x < 0 || x >= dev->width)
        return;
    unsigned char c = 0x80 >> (x & 7);
    if (backg)
        *(base + ((x >> 3) + y*bytpline)) &= ~c;
    else
        *(base + ((x >> 3) + y*bytpline)) |= c;
}


// Draw multiple pixels.
//
void
RASparams::Pixels(GRmultiPt *data, int n)
{
    if (n < 1)
        return;
    data->data_ptr_init();
    while (n--) {
        int x = data->data_ptr_x();
        int y = data->data_ptr_y();
        data->data_ptr_inc();
        if (y < 0 || y >= dev->height || x < 0 || x >= dev->width)
            continue;
        unsigned char c = 0x80 >> (x & 7);
        if (backg)
            *(base + ((x >> 3) + y*bytpline)) &= ~c;
        else
            *(base + ((x >> 3) + y*bytpline)) |= c;
    }
}


#define ror(b) b >>= 1; if (!b) b = 0x80
#define rorl(b, p) b >>= 1; if (!b) b = 1 << (p-1)

// Draw line function.
//
void
RASparams::Line(int x1, int y1, int x2, int y2)
{
    if (dev->LineClip(&x1, &y1, &x2, &y2))
        return;
    if (x2 < x1) {
        swap(x1, x2);
        swap(y1, y2);
    }
    int dx = x2 - x1;
    int dy = y2 - y1;

    int lcnt = y1;
    int next = bytpline;
    char *rgen = (char*)base + (x1 >> 3) + lcnt*next;

    if (y1 > y2) {
        next = -next;
        dy = -dy;
    }
    int dy2 = dy;

    unsigned right = linestyle;
    if (!right)
        right = 0xff;
    unsigned int left;
    int cnt;
    for (left = right, cnt = 0; left; left >>= 1, cnt++) ;
    left = 1 << (cnt - 1 - (lcnt + x1)%cnt);
    unsigned char cbuf = 0x80 >> (x1 & 7);

    int errterm = -dx >> 1;
    for (dy++; dy; dy--) {
        errterm += dx;
        if (errterm <= 0) {
            if (left & right) {
                if (backg)
                    *rgen &= ~cbuf;
                else
                    *rgen |= cbuf;
            }
            rgen += next;
            rorl(left, cnt);
            continue;
        }
        while (errterm > 0 && x1 != x2) {
            if (left & right) {
                if (backg)
                    *rgen &= ~cbuf;
                else
                    *rgen |= cbuf;
            }
            rorl(left, cnt);
            ror(cbuf);
            if (cbuf & 0x80) rgen++;
            x1++;
            errterm -= dy2;
        }
        rgen += next;
    }
}


// Draw a path.
//
void
RASparams::PolyLine(GRmultiPt *data, int n)
{
    if (n < 2)
        return;
    data->data_ptr_init();
    n--;
    while (n--) {
        int x = data->data_ptr_x();
        int y = data->data_ptr_y();
        data->data_ptr_inc();
        Line(x, y, data->data_ptr_x(), data->data_ptr_y());
    }
}


// Draw multiple lines.
//
void
RASparams::Lines(GRmultiPt *data, int n)
{
    if (n < 1)
        return;
    data->data_ptr_init();
    while (n--) {
        int x = data->data_ptr_x();
        int y = data->data_ptr_y();
        data->data_ptr_inc();
        Line(x, y, data->data_ptr_x(), data->data_ptr_y());
        data->data_ptr_inc();
    }
}


// Draw a rectangle, possibly filled.
//
void
RASparams::Box(int xl, int yl, int xu, int yu)
{
    if (yu < yl) swap(yu, yl);
    if (xu < xl) swap(xu, xl);

    if (xl < 0)
        xl = 0;
    else if (xl >= dev->width)
        return;

    if (xu > dev->width - 1)
        xu = dev->width - 1;
    else if (xu < 0)
        return;

    if (yl < 0)
        yl = 0;
    else if (yl >= dev->height)
        return;

    if (yu > dev->height - 1)
        yu = dev->height - 1;
    else if (yu < 0)
        return;

    int lnum = yl;
    // The box includes the boundary points, so will cover a line
    // box with the same points.
    int dy = yu - yl + 1;
    xu++;

    unsigned char left  =  (0xff >> (xl & 7));
    unsigned char right = ~(0xff >> (xu & 7));
    int dx = (xu >> 3) - (xl >>= 3) - 1;
    if (dx < 0) { left &= right; dx = 0; right = 0; }
    unsigned char *rgen = base + xl + (long) lnum*bytpline;
    int next = bytpline - 1 - dx;

    if (!curfillpatt) {
        while (dy--) {
            if (backg)
                *rgen &= ~left;
            else
                *rgen |= left;
            rgen++;
            int tx = dx;
            while (tx--) {
                if (backg)
                    *rgen = 0;
                else
                    *rgen = 0xff;
                rgen++;
            }
            if (backg)
                *rgen &= ~right;
            else
                *rgen |= right;
            rgen += next;
        }
    }
    else {
        const GRfillType *fpat = curfillpatt;
        while (dy--) {
            unsigned int xo = xl;
            unsigned int yo = lnum++;
            fpat->initByteScan(&xo, &yo);
            unsigned char c = fpat->getNextColByte(&xo, yo);
            if (backg)
                *rgen &= ~(c & left);
            else
                *rgen |= (c & left);
            rgen++;
            int tx = dx;
            while (tx--) {
                c = fpat->getNextColByte(&xo, yo);
                if (backg)
                    *rgen &= ~c;
                else
                    *rgen |= c;
                rgen++;
            }
            c = fpat->getNextColByte(&xo, yo);
            if (backg)
                *rgen &= ~(c & right);
            else
                *rgen |= (c & right);
            rgen += next;
        }
    }
}


// Draw multiple boxes.
//
void
RASparams::Boxes(GRmultiPt *data, int n)
{
    if (n < 1)
        return;
    data->data_ptr_init();
    while (n--) {
        int x = data->data_ptr_x();
        int y = data->data_ptr_y();
        data->data_ptr_inc();
        int w = data->data_ptr_x();
        int h = data->data_ptr_y();
        data->data_ptr_inc();
        Box(x, y, x + w - 1, y + h - 1);
    }
}



// Draw an arc.
//
void
RASparams::Arc(int x, int y, int rx, int ry, double theta1, double theta2)
{
    if (rx <= 0 || ry <= 0)
        return;
    while (theta1 >= theta2)
        theta2 = 2*M_PI + theta2;
    double dt = M_PI/100;
    int x0 = (int)(rx*cos(theta1));
    int y0 = -(int)(ry*sin(theta1));
    for (double ang = theta1 + dt; ang < theta2; ang += dt) {
        int x1 = (int)(rx*cos(ang));
        int y1 = -(int)(ry*sin(ang));
        Line(x+x0, y+y0, x+x1, y+y1);
        x0 = x1;
        y0 = y1;
    }
    Line(x+x0, y+y0, x+(int)(rx*cos(theta2)), y-(int)(ry*sin(theta2)));
}


// Draw a polygon, possibly filled.
//
void
RASparams::Polygon(GRmultiPt *data, int num)
{
    poly_decomp::instance().polygon(this, data, num);
}


// Draw a trapezoid.
//
void
RASparams::Zoid(int yl, int yu, int xll, int xul, int xlr, int xur)
{
    if (yl >= dev->height || yu < 0)
        return;

    int dy = yu - yl;
    int next = bytpline;
    unsigned char *rgen1 = base + yl*next;

    unsigned char *rgen0 = rgen1;
    int dx1 = xul - xll;
    int dx2 = xlr - xur;
    int s1 = 1;
    int s2 = -1;
    if (xll >= xul) { dx1 = -dx1; s1 = -1; }
    if (xur >= xlr) { dx2 = -dx2; s2 = 1; }
    int errterm1 = -dx1 >> 1;
    int errterm2 = -dx2 >> 1;
    xlr++;
    xur++;

    if (!curfillpatt) {
        for (int y = yl; y <= yu; y++) {
            if (y >= 0 && y < dev->height && xll < dev->width && xlr >= 0) {
                int xllc = xll;
                if (xllc < 0)
                    xllc = 0;
                int xlrc = xlr;
                if (xlrc >= dev->width)
                    xlrc = dev->width - 1;
                unsigned char left  =  (0xff >> (xllc & 7));
                unsigned char right = ~(0xff >> (xlrc & 7));
                int dx = (xlrc >> 3) - (xllc >> 3) - 1;
                if (dx < 0) { left &= right; dx = 0; right = 0; }
                unsigned char *rgen = rgen0 + (xllc >> 3);
                if (backg)
                    *rgen &= ~left;
                else
                    *rgen |= left;
                rgen++;
                while (dx--) {
                    if (backg)
                        *rgen = 0;
                    else
                        *rgen = 0xff;
                    rgen++;
                }
                if (backg)
                    *rgen &= ~right;
                else
                    *rgen |= right;
            }
            rgen0 += next;

            errterm1 += dx1;
            errterm2 += dx2;
            while (errterm1 > 0 && xul != xll) {
                errterm1 -= dy;
                xll += s1;
            }
            while (errterm2 > 0 && xlr != xur) {
                errterm2 -= dy;
                xlr += s2;
            }
        }
    }
    else {
        const GRfillType *fpat = curfillpatt;
        for (int y = yl; y <= yu; y++) {
            if (y >= 0 && y < dev->height && xll < dev->width && xlr >= 0) {
                int xllc = xll;
                if (xllc < 0)
                    xllc = 0;
                int xlrc = xlr;
                if (xlrc >= dev->width)
                    xlrc = dev->width - 1;
                unsigned char left  =  (0xff >> (xllc & 7));
                unsigned char right = ~(0xff >> (xlrc & 7));
                int dx = (xlrc >> 3) - (xllc >> 3) - 1;
                if (dx < 0) { left &= right; dx = 0; right = 0; }
                unsigned char *rgen = rgen0 + (xllc >>= 3);
                unsigned int xo = xllc;
                unsigned int yo = y;
                fpat->initByteScan(&xo, &yo);
                unsigned char c = fpat->getNextColByte(&xo, yo);
                if (backg)
                    *rgen &= ~(c & left);
                else
                    *rgen |= (c & left);
                rgen++;
                while (dx--) {
                    c = fpat->getNextColByte(&xo, yo);
                    if (backg)
                        *rgen = ~c;
                    else
                        *rgen = c;
                    rgen++;
                }
                c = fpat->getNextColByte(&xo, yo);
                if (backg)
                    *rgen &= ~(c & right);
                else
                    *rgen |= (c & right);
            }
            rgen0 += next;

            errterm1 += dx1;
            errterm2 += dx2;
            while (errterm1 > 0 && xul != xll) {
                errterm1 -= dy;
                xll += s1;
            }
            while (errterm2 > 0 && xlr != xur) {
                errterm2 -= dy;
                xlr += s2;
            }
        }
    }
}


// Render a text string.  We don't handle transforms here.
//
void
RASparams::Text(const char *text, int x, int y, int xform, int width,
    int height)
{
    if (!width || !height)
        return;
    int lwid, lhei, numlines;
    FT.textExtent(text, &lwid, &lhei, &numlines);
    int w, h;
    if (width > 0 && height > 0) {
        w = width/lwid;
        h = height/lhei;
    }
    else {
        w = (int)(lwid*text_scale);
        h = (int)(lhei*text_scale);
    }
    FT.renderText(this, text, x, y, xform, w, h);
}


// Get the pixel extent of string.
//
void
RASparams::TextExtent(const char *text, int *x, int *y)
{
    int lwid, lhei, lnum;
    FT.textExtent(text, &lwid, &lhei, &lnum);
    *x = (int)(lwid*text_scale);
    *y = (int)(lhei*text_scale);
}


// Set linestyle function.
//
void
RASparams::SetLinestyle(const GRlineType *lineptr)
{
    if (!lineptr || !lineptr->mask || lineptr->mask == -1)
        linestyle = 0;
    else
        linestyle = lineptr->mask;
}


// Set fillpattern function.
//
void
RASparams::SetFillpattern(const GRfillType *fillp)
{
    if (!fillp || !fillp->hasMap())
        curfillpatt = 0;
    else
        curfillpatt = fillp;
}


void
RASparams::SetColor(int pixel)
{
    if (pixel == GRappIf()->BackgroundPixel())
        backg = true;
    else
        backg = false;
}


void
RASparams::DisplayImage(const GRimage *image, int x, int y,
    int width, int height)
{
    for (int i = 0; i < height; i++) {
        int yd = i + y;
        if (yd < 0)
            continue;
        if ((unsigned int)yd >= image->height())
            break;
        for (int j = 0; j < width; j++) {
            int xd = j + x;
            if (xd < 0)
                continue;
            if ((unsigned int)xd >= image->width())
                break;
            unsigned int pix = image->data()[xd + yd*image->width()];
            put_pixel(pix, xd, yd);
        }
    }
}


void
RASparams::put_pixel(unsigned int pix, int x, int y)
{
    if (y < 0 || y >= dev->height || x < 0 || x >= dev->width)
        return;
    unsigned char c = 0x80 >> (x & 7);
    if (pix & 0xffffff)
        *(base + ((x >> 3) + y*bytpline)) |= c;
    else
        *(base + ((x >> 3) + y*bytpline)) &= ~c;
}
