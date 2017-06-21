
/*=======================================================================*
 *                                                                       *
 *  XICTOOLS Integrated Circuit Design System                            *
 *  Copyright (c) 2005 Whiteley Research Inc, all rights reserved.       *
 *                                                                       *
 * MOZY html viewer application files                                    *
 *                                                                       *
 * Based on previous work identified below.                              *
 *-----------------------------------------------------------------------*
 * This file is part of the gtkhtm widget library.  The gtkhtm library
 * was derived from the gtk-xmhtml library by:
 *
 *   Stephen R. Whiteley  <srw@wrcad.com>
 *   Whiteley Research Inc.
 *-----------------------------------------------------------------------*
 *  The gtk-xmhtml widget was derived from the XmHTML library by
 *  Miguel de Icaza  <miguel@nuclecu.unam.mx> and others from the GNOME
 *  project.
 *  11/97 - 2/98
 *-----------------------------------------------------------------------*
 * Author:  newt
 * (C)Copyright 1995-1996 Ripley Software Development
 * All Rights Reserved
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *-----------------------------------------------------------------------*
 * $Id: htm_parser.h,v 1.2 2007/08/25 07:22:31 stevew Exp $
 *-----------------------------------------------------------------------*/

#ifndef HTM_PARSER_H
#define HTM_PARSER_H

namespace htm
{
    // Error codes for htmParser::callback
    //
    enum parserError
    {
        HTML_NONE,              // no error
        HTML_UNKNOWN_ELEMENT,   // unknown HTML element
        HTML_BAD,               // very badly placed element
        HTML_OPEN_BLOCK,        // block still open while new block started
        HTML_CLOSE_BLOCK,       // block closed but was never opened
        HTML_OPEN_ELEMENT,      // unbalanced terminator
        HTML_NESTED,            // improperly nested element
        HTML_VIOLATION,         // bad content for current block/element
        HTML_NOTIFY,            // notification of text insertion/removal
        HTML_INTERNAL           // internal parser error
    };

    // Parser state stack object
    //
    struct stateStack
    {
        stateStack(htmlEnum i = HT_DOCTYPE) { id = i; next = 0; }

        htmlEnum id;                    // current state id
        struct stateStack *next;        // ptr to next record
    };
}

// Representation of parsed HTML elements
//
struct htmObject
{
    htmObject(htmlEnum i, char *e, const char *a, bool end, bool term, int l)
        {
            id          = i;
            element     = e;
            attributes  = a;
            is_end      = end;
            terminated  = term;
            line        = l;
            next = prev = 0;
        }

    ~htmObject()
        {
            delete [] element;
        }

    void free()
        {
            htmObject *objects = this;
            while (objects) {
                htmObject *nxt = objects->next;
                delete objects;
                objects = nxt;
            }
        }

    htmlEnum id;                // internal ID for this element
    char *element;              // element text
    const char *attributes;     // element attributes (if any)
    bool is_end;                // true if this is a closing element
    bool terminated;            // true if element has closing counterpart
    unsigned int line;          // line number in input for this element
    htmObject *next;
    htmObject *prev;
};

struct htmParser
{
    htmParser(htmWidget*);
    ~htmParser();

    void setSource(const char*);
    htmObject *parse(const char*);
    htmObject *verifyVerify(htmObject*);

    void setWarnings(int w) { p_warn = w; }
    bool get_bad_html() { return (p_bad_html); }
    bool get_html32() { return (p_html32); }

private:
    void parseHTML();
    void parsePerfectHTML();
    void parsePLAIN();
    void parseIMAGE();

    int verify(htmlEnum, bool);
    void callback(htmlEnum, htmlEnum, parserError);
    void warning(htmlEnum, htmlEnum, parserError);

    int checkElementOccurance(htmlEnum, htmlEnum);
    bool checkElementContent(htmlEnum, htmlEnum);
    int tokenToId(const char*, bool);
    void pushState(htmlEnum);
    htmlEnum popState();
    bool onStack(htmlEnum);

    htmObject* newObject(htmlEnum, char*, char*, bool, bool);
    void insertElement(const char*, htmlEnum, bool);
    bool terminateElement(const char*, htmlEnum, htmlEnum);
    void copyElement(htmObject*, bool);
    char *storeElement(char*, char*);
    char *storeElementUnconditional(char*, char*);
    void storeTextElement(char*, char*, bool = false);
    char *cutComment(char *start);
    void warning(const char*, const char*, ...);

    htmWidget *p_html;            // widget for callbacks
    char *p_source;               // text being parsed
    int p_len;                    // length of input text
    int p_index;                  // last known position
    int p_num_lines;              // current line count
    unsigned p_line_len;          // maximum line length so far
    unsigned p_cnt;               // current line length
    int p_num_elements;           // no of tags inserted so far
    int p_num_text;               // no of text elements inserted so far
    htmObject *p_head;            // head of object list
    htmObject *p_current;         // lastly inserted element
    stateStack p_state_base;      // stack base point
    stateStack *p_state_stack;    // actual stack
    int p_cstart;                 // current element start position
    int p_cend;                   // current element end position
    int p_inserted;               // no of auto-inserted chars
    unsigned p_err_count;         // no of errors so far
    unsigned p_loop_count;        // no of loops made so far
    bool p_have_body;             // indicates presence of <body> tag

    // returns
    bool p_bad_html;              // bad HTML document flag
    bool p_html32;                // HTML32 conforming document flag

    // input
    unsigned char p_warn;         // warn about bad html constructs
    bool p_directionRtoL;         // right-to-left text
};

#endif
