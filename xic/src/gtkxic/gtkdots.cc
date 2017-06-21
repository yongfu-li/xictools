
/*========================================================================*
 *                                                                        *
 *  XICTOOLS Integrated Circuit Design System                             *
 *  Copyright (c) 2007 Whiteley Research Inc, all rights reserved.        *
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
 $Id: gtkdots.cc,v 5.13 2016/03/02 00:39:40 stevew Exp $
 *========================================================================*/

#include "main.h"
#include "sced.h"
#include "gtkmain.h"
#include "gtkinlines.h"
#include "menu.h"
#include "attr_menu.h"


//-------------------------------------------------------------------------
// Pop-up to control electrical connection point display

namespace {
    namespace gtkdots {
        struct sDt
        {
            sDt(GRobject);
            ~sDt();

            void update();

            GtkWidget *shell() { return (dt_popup); }

        private:
            static void dt_cancel_proc(GtkWidget*, void*);
            static void dt_action(GtkWidget*, void*);

            GRobject dt_caller;
            GtkWidget *dt_popup;
            GtkWidget *dt_none;
            GtkWidget *dt_norm;
            GtkWidget *dt_all;
        };

        sDt *Dt;
    }
}

using namespace gtkdots;


void
cSced::PopUpDots(GRobject caller, ShowMode mode)
{
    if (!GRX || !mainBag())
        return;
    if (mode == MODE_OFF) {
        delete Dt;
        return;
    }
    if (mode == MODE_UPD) {
        if (Dt)
            Dt->update();
        return;
    }
    if (Dt)
        return;

    new sDt(caller);
    if (!Dt->shell()) {
        delete Dt;
        return;
    }
    gtk_window_set_transient_for(GTK_WINDOW(Dt->shell()),
        GTK_WINDOW(mainBag()->Shell()));

    GRX->SetPopupLocation(GRloc(), Dt->shell(), mainBag()->Viewport());
    gtk_widget_show(Dt->shell());
}


sDt::sDt(GRobject caller)
{
    Dt = this;
    dt_caller = caller;
    dt_none = 0;
    dt_norm = 0;
    dt_all = 0;

    dt_popup = gtk_NewPopup(0, "Connection Points", dt_cancel_proc, 0);
    if (!dt_popup)
        return;
    gtk_window_set_resizable(GTK_WINDOW(dt_popup), false);

    GtkWidget *form = gtk_table_new(1, 1, false);
    gtk_widget_show(form);
    gtk_container_set_border_width(GTK_CONTAINER(form), 2);
    gtk_container_add(GTK_CONTAINER(dt_popup), form);
    int rowcnt = 0;

    dt_none = gtk_radio_button_new_with_label(0, "Don't show dots");
    gtk_widget_set_name(dt_none, "none");
    gtk_widget_show(dt_none);
    GSList *group = gtk_radio_button_group(GTK_RADIO_BUTTON(dt_none));
    gtk_signal_connect(GTK_OBJECT(dt_none), "clicked",
        GTK_SIGNAL_FUNC(dt_action), 0);

    gtk_table_attach(GTK_TABLE(form), dt_none, 0, 1, rowcnt, rowcnt+1,
        (GtkAttachOptions)(GTK_EXPAND | GTK_FILL | GTK_SHRINK),
        (GtkAttachOptions)0, 2, 2);
    rowcnt++;

    dt_norm = gtk_radio_button_new_with_label(group, "Show dots normally");
    gtk_widget_set_name(dt_norm, "norm");
    gtk_widget_show(dt_norm);
    group = gtk_radio_button_group(GTK_RADIO_BUTTON(dt_norm));
    gtk_signal_connect(GTK_OBJECT(dt_norm), "clicked",
        GTK_SIGNAL_FUNC(dt_action), 0);

    gtk_table_attach(GTK_TABLE(form), dt_norm, 0, 1, rowcnt, rowcnt+1,
        (GtkAttachOptions)(GTK_EXPAND | GTK_FILL | GTK_SHRINK),
        (GtkAttachOptions)0, 2, 2);
    rowcnt++;

    dt_all = gtk_radio_button_new_with_label(group,
        "Show dot at every connection");
    gtk_widget_set_name(dt_all, "all");
    gtk_widget_show(dt_all);
    gtk_signal_connect(GTK_OBJECT(dt_all), "clicked",
        GTK_SIGNAL_FUNC(dt_action), 0);

    gtk_table_attach(GTK_TABLE(form), dt_all, 0, 1, rowcnt, rowcnt+1,
        (GtkAttachOptions)(GTK_EXPAND | GTK_FILL | GTK_SHRINK),
        (GtkAttachOptions)0, 2, 2);
    rowcnt++;

    GtkWidget *sep = gtk_hseparator_new();
    gtk_widget_show(sep);
    gtk_table_attach(GTK_TABLE(form), sep, 0, 1, rowcnt, rowcnt+1,
        (GtkAttachOptions)(GTK_EXPAND | GTK_FILL | GTK_SHRINK),
        (GtkAttachOptions)0, 2, 2);
    rowcnt++;

    GtkWidget *button = gtk_button_new_with_label("Dismiss");
    gtk_widget_set_name(button, "Dismiss");
    gtk_widget_show(button);
    gtk_signal_connect(GTK_OBJECT(button), "clicked",
        GTK_SIGNAL_FUNC(dt_cancel_proc), 0);

    gtk_table_attach(GTK_TABLE(form), button, 0, 1, rowcnt, rowcnt+1,
        (GtkAttachOptions)(GTK_EXPAND | GTK_FILL | GTK_SHRINK),
        (GtkAttachOptions)(GTK_EXPAND | GTK_FILL | GTK_SHRINK), 2, 2);
    gtk_window_set_focus(GTK_WINDOW(dt_popup), button);

    // Constrain overall widget width so title text isn't truncated.
    gtk_widget_set_usize(button, 240, -1);
    update();
}


sDt::~sDt()
{
    Dt = 0;
    if (dt_caller)
        GRX->SetStatus(dt_caller, false);
    if (dt_popup)
        gtk_widget_destroy(dt_popup);
}


void
sDt::update()
{
    const char *v = CDvdb()->getVariable(VA_ShowDots);
    if (!v) {
        GRX->SetStatus(dt_none, true);
        GRX->SetStatus(dt_norm, false);
        GRX->SetStatus(dt_all, false);
    }
    else if (*v == 'a' || *v == 'A') {
        GRX->SetStatus(dt_none, false);
        GRX->SetStatus(dt_norm, false);
        GRX->SetStatus(dt_all, true);
    }
    else {
        GRX->SetStatus(dt_none, false);
        GRX->SetStatus(dt_norm, true);
        GRX->SetStatus(dt_all, false);
    }
}


// Static Function
void
sDt::dt_cancel_proc(GtkWidget*, void*)
{
    SCD()->PopUpDots(0, MODE_OFF);
}


// Static function.
void
sDt::dt_action(GtkWidget *caller, void*)
{
    if (!Dt)
        return;
    if (!GRX->GetStatus(caller))
        return;
    if (caller == Dt->dt_none)
        CDvdb()->clearVariable(VA_ShowDots);
    else if (caller == Dt->dt_norm)
        CDvdb()->setVariable(VA_ShowDots, "");
    else if (caller == Dt->dt_all)
        CDvdb()->setVariable(VA_ShowDots, "all");
}
