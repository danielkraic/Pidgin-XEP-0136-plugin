#define PURPLE_PLUGINS

/*#include <glib.h>*/
#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include "connection.h"
#include "debug.h"
#include "gtksourceundomanager.h"
#include "internal.h"
#include "notify.h"
#include "plugin.h"
#include "prpl.h"
#include "xmlnode.h"
#include "version.h"

#include <gtkconv.h>
#include <gtkimhtml.h>
#include <gtkplugin.h>
#include <version.h>
#include "gtkutils.h"

/*
 * -------------------------------------------------- 
 *            Pidgin XEP-136 plugin
 * -------------------------------------------------- 
 */
static PurplePlugin *xep136 = NULL;

typedef struct _WindowStruct {
    GtkWidget *window;
    GtkWidget *hbox;
    GtkWidget *vbox;
    GtkWidget *imhtml;
    GtkWidget *label;
    GtkWidget *entry;
    GtkWidget *button;
    PidginConversation *gtkconv;
} WindowStruct;

WindowStruct *history_window = NULL;

static void
history_window_destroy(GtkWidget *button, gpointer null)
{
    g_free(history_window);
    history_window = NULL;
}

static void
history_window_create(GtkWidget *button, PidginConversation *gtkconv)
{
    // ked uz existuje nerobi nic
    if (history_window)
	return;

    history_window = g_new0(WindowStruct, 1);

    history_window->window = pidgin_create_window(_("XEP-136 History"), PIDGIN_HIG_BORDER, NULL, TRUE);
    gtk_window_set_default_size(GTK_WINDOW(history_window->window), 300, 350);

    g_signal_connect(G_OBJECT(history_window->window), "destroy", 
	    G_CALLBACK(history_window_destroy), NULL);

    history_window->imhtml = gtk_imhtml_new(NULL, NULL);
    history_window->label = gtk_label_new("Search ");
    history_window->entry = gtk_entry_new();
    history_window->button = gtk_button_new_from_stock(GTK_STOCK_FIND);

    history_window->hbox = gtk_hbox_new(FALSE, 3);
    history_window->vbox = gtk_vbox_new(FALSE, 3);

    gtk_box_pack_start(GTK_BOX(history_window->vbox), 
		pidgin_make_scrollable(history_window->imhtml, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC, GTK_SHADOW_ETCHED_IN, -1, -1),
		TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(history_window->vbox), history_window->hbox, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(history_window->hbox), history_window->label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(history_window->hbox), history_window->entry, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(history_window->hbox), history_window->button, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(history_window->window), history_window->vbox);
    gtk_widget_show_all(history_window->window);

}

static gboolean
history_off(PidginConversation *gtkconv)
{
    //TODO vypnut 
    
    return TRUE;
}


static gboolean
history_on(PidginConversation *gtkconv)
{
    // TODO zapnut
    
    return TRUE;
}

static int
history_check(GtkToggleButton *check, PidginConversation *gtkconv)
{
    PurpleAccount *acc;
    PurpleConversation *conv;
    char *jabber_id = "prpl-jabber";
    int res = 3; 				// default error

    conv = gtkconv->active_conv;

    acc = conv->account;
    
    // id must be prpl-jabber
    if (g_strcmp0(jabber_id, purple_account_get_protocol_id(acc)))
	return 2;		// not supported

    if(gtk_toggle_button_get_active(check)) {
	// toggle button active
	if (history_on(gtkconv))
	    res = 0;		// enabled
	else 
	    res = 2;
    }
    else {
	// toggle button not active
	if (history_off(gtkconv))
	    res = 1;		// disabled
	else 
	    res = 2;
    }
   
    return res;
}

static void
history_change(GtkToggleButton *check, PidginConversation *gtkconv)
{
    PurpleConversation *conv;
    PurpleConvIm *im;
    int res = 3; 				// default error
    char *text;
    char *message[] = {
	"XEP-136: message archiving enabled!", 	// 0 enabled
	"XEP-136: message archiving disabled!", // 1 disabled
	"XEP-136: not supported!",		// 2 not supported
	"XEP-136: Error!"			// 3 error
    };

    res = history_check(check, gtkconv);
    text = message[res];

    conv = gtkconv->active_conv;
    g_return_if_fail(conv != NULL);

    im = PURPLE_CONV_IM(conv); 

    purple_conv_im_write(im, NULL, text, PURPLE_MESSAGE_SYSTEM, time(NULL));
}

static void
detach_from_gtkconv(PidginConversation *gtkconv, gpointer null)
{
    GtkWidget *toolbar_box, *hbox;

    toolbar_box = gtkconv->toolbar;

    hbox = g_object_get_data(G_OBJECT(toolbar_box), "xep136_hbox");

    if (hbox)
	gtk_container_remove(GTK_CONTAINER(toolbar_box), hbox);
    
    gtk_widget_queue_draw(pidgin_conv_get_window(gtkconv)->window);
}

static void
attach_to_gtkconv(PidginConversation *gtkconv, gpointer null)
{
    GtkWidget *toolbar_box, *hbox, *check, *button;

    toolbar_box = gtkconv->toolbar;

    check = gtk_check_button_new();
    button = gtk_button_new_with_label("History");

    g_signal_connect(G_OBJECT(check), "toggled",
	    G_CALLBACK(history_change), (gpointer) gtkconv);

    g_signal_connect(G_OBJECT(button), "clicked",
	    G_CALLBACK(history_window_create), (gpointer) gtkconv);

    hbox = gtk_hbox_new(FALSE, 5);
    gtk_box_pack_start(GTK_BOX(hbox), check, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
    g_object_set_data(G_OBJECT(toolbar_box), "xep136_hbox", hbox);

    gtk_box_pack_end(GTK_BOX(toolbar_box), hbox, FALSE, FALSE, 0);

    gtk_widget_show_all(hbox);

    gtk_widget_queue_draw(pidgin_conv_get_window(gtkconv)->window);
}

static void
detach_from_pidgin_window(PidginWindow *win, gpointer null)
{
    g_list_foreach(pidgin_conv_window_get_gtkconvs(win), (GFunc) detach_from_gtkconv, NULL);
}

static void
attach_to_pidgin_window(PidginWindow *win, gpointer null)
{
    g_list_foreach(pidgin_conv_window_get_gtkconvs(win), (GFunc) attach_to_gtkconv, NULL);
}

static void
detach_from_all_windows()
{
    g_list_foreach(pidgin_conv_windows_get_list(), (GFunc) detach_from_pidgin_window, NULL);
}

static void
attach_to_all_windows()
{
    g_list_foreach(pidgin_conv_windows_get_list(), (GFunc) attach_to_pidgin_window, NULL);
}

static void
conv_created_cb(PurpleConversation *conv, gpointer null)
{
    PidginConversation *gtkconv = PIDGIN_CONVERSATION(conv);

    g_return_if_fail(gtkconv != NULL);

    attach_to_gtkconv(gtkconv, NULL);
}

static gboolean
plugin_load(PurplePlugin *plugin)
{
    xep136 = plugin;

    attach_to_all_windows();

    purple_signal_connect(purple_conversations_get_handle(), 
	    "conversation-created",
	    plugin, PURPLE_CALLBACK(conv_created_cb), NULL);

    return TRUE;
}

static gboolean
plugin_unload(PurplePlugin *plugin)
{
    detach_from_all_windows();

    return TRUE;
}

static PurplePluginInfo info = {
    PURPLE_PLUGIN_MAGIC,
    PURPLE_MAJOR_VERSION,
    PURPLE_MINOR_VERSION,
    PURPLE_PLUGIN_STANDARD,
    NULL,
    0,
    NULL,
    PURPLE_PRIORITY_DEFAULT,

    "gtk-daniel_kraic-xep136_plugin",
    "XEP-136 plugin",
    "1.2",

    "XEP-136 plugin",
    "XEP-136 plugin",
    "Daniel Kraic <danielkraic@gmail.com>",
    "https://github.com/danielkraic/Pidgin-XEP-0136-plugin",

    plugin_load,
    plugin_unload,

    NULL,
    NULL,
    NULL,
    NULL,
    NULL, /*    plugin_actions, */
    /* this tells libpurple the address of the function to call
                       to get the list of plugin actions. */
    NULL,
    NULL,
    NULL,
    NULL
};

static void
init_plugin(PurplePlugin *plugin)
{
}

PURPLE_INIT_PLUGIN(xep136_plugin, init_plugin, info)
