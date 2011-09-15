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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PLUGIN_ID "gtk-daniel_kraic-xep136_plugin" 

/*
 * -------------------------------------------------- 
 *            Pidgin XEP-136 plugin
 * -------------------------------------------------- 
 */

static PurplePlugin *xep136 = NULL;

typedef struct _WindowStruct {
    GtkWidget *window;
    GtkWidget *mainbox;
    GtkWidget *rightbox;
    GtkWidget *hbox;
    GtkWidget *vbox;
    GtkWidget *imhtml;
    GtkWidget *label;
    GtkWidget *entry;
    GtkWidget *button;
    GtkWidget *show;
    GtkWidget *next;
    GtkWidget *enable;
    GtkWidget *disable;
    PidginConversation *gtkconv;
} WindowStruct;

WindowStruct *history_window = NULL;

static void
message_send(char *message, PurpleConnection *gc)
{
    PurplePluginProtocolInfo *prpl_info = NULL;

    if (gc)
	    prpl_info = PURPLE_PLUGIN_PROTOCOL_INFO(gc->prpl);

    if (prpl_info && prpl_info->send_raw != NULL)
	    prpl_info->send_raw(gc, message, strlen(message));
}

static void
xmlnode_received(PurpleConnection *gc, xmlnode **packet, gpointer null)
{
    xmlnode *xml = *packet;
    xmlnode *c = NULL;
    xmlnode *d = NULL;
    char *xmlns = "http://www.xmpp.org/extensions/xep-0136.html#ns";

	if (!history_window /*|| console->gc != gc*/)
		return;

	if (strcmp(xml->name, "iq") == 0) {
	    for (c = xml->child; c; c = c->next) {
		if (strcmp(c->name, "query") == 0) {
		    for (d = c->child; d; d = d->next) {
			if ( (strcmp(d->name, "feature") == 0) && (strcmp( (d->child)->name, "var" ) == 0) ) {
			    //gtk_imhtml_append_text(GTK_IMHTML(history_window->imhtml), (d->child)->data, 0);
			    //gtk_imhtml_append_text(GTK_IMHTML(history_window->imhtml), "<br>", 0);
			    if (strcmp(( d->child)->data, xmlns) == 0) {

				gtk_widget_set_sensitive(history_window->rightbox, TRUE);
				gtk_imhtml_append_text(GTK_IMHTML(history_window->imhtml), "XEP-0136 supported!", 0);
				gtk_imhtml_append_text(GTK_IMHTML(history_window->imhtml), "<br>", 0);
			    }
			}
		    }
		}
	    }
	}
}

static void
history_window_destroy(GtkWidget *button, gpointer null)
{
    g_free(history_window);
    history_window = NULL;
}

static gchar * 
get_server_name(gchar *username)
{
    gchar *server = NULL;
    gchar *zav = NULL;
    gchar *lom = NULL;

    glong dlzka = g_utf8_strlen(username, -1);

    zav = g_strstr_len(username, dlzka, "@");
    lom = g_strstr_len(username, dlzka, "/");

    zav++;

    if (lom == NULL)
    {
	dlzka = g_utf8_strlen(zav, -1);
    } else {
	dlzka = g_utf8_strlen(zav, -1) - g_utf8_strlen(lom, -1);
    }
 
    server = (gchar *) g_malloc0( (dlzka + 1) * sizeof(gchar) );

    g_strlcat (server, zav, dlzka + 1);

    return server;
}


static void
history_window_create(void)
{
    history_window = g_new0(WindowStruct, 1);

    history_window->window = pidgin_create_window(_("XEP-136 History"), PIDGIN_HIG_BORDER, NULL, TRUE);
    gtk_window_set_default_size(GTK_WINDOW(history_window->window), 400, 350);

    g_signal_connect(G_OBJECT(history_window->window), "destroy", 
	    G_CALLBACK(history_window_destroy), NULL);

    history_window->imhtml = gtk_imhtml_new(NULL, NULL);
    history_window->label = gtk_label_new("Search ");
    history_window->entry = gtk_entry_new();
    history_window->button = gtk_button_new_from_stock(GTK_STOCK_FIND);

    history_window->show = gtk_button_new_with_label("Show");
    history_window->next = gtk_button_new_with_label("Next");
    history_window->enable = gtk_button_new_with_label("Enable");
    history_window->disable = gtk_button_new_with_label("Disable");

    history_window->mainbox = gtk_hbox_new(FALSE, 3);
    history_window->rightbox = gtk_vbox_new(FALSE, 3);

    history_window->hbox = gtk_hbox_new(FALSE, 3);
    history_window->vbox = gtk_vbox_new(FALSE, 3);

    gtk_box_pack_start(GTK_BOX(history_window->mainbox), history_window->vbox, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(history_window->mainbox), history_window->rightbox, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(history_window->vbox), 
		pidgin_make_scrollable(history_window->imhtml, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC, GTK_SHADOW_ETCHED_IN, -1, -1),
		TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(history_window->vbox), history_window->hbox, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(history_window->hbox), history_window->label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(history_window->hbox), history_window->entry, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(history_window->hbox), history_window->button, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(history_window->rightbox), history_window->show, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(history_window->rightbox), history_window->next, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(history_window->rightbox), history_window->enable, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(history_window->rightbox), history_window->disable, FALSE, FALSE, 0);

    gtk_widget_set_sensitive(history_window->rightbox, FALSE);

    gtk_container_add(GTK_CONTAINER(history_window->window), history_window->mainbox);
    gtk_widget_show_all(history_window->window);
}

static void
history_window_open(GtkWidget *button, PidginConversation *gtkconv)
{
    gchar *message = NULL;

    PurpleConversation *purple_conv = gtkconv->active_conv;
    PurpleAccount *acc = purple_conv->account;
    PurpleConnection *gc = acc->gc;

    char *username= acc->username;
    char *server = NULL;

 // ked uz existuje nerobi nic
    if (history_window)
	return;

    server = get_server_name(username);

    if (server == NULL) {
	purple_debug_misc(PLUGIN_ID, "history_window_open :: ER server\n");
	return;
    }

    if (!gc) {
	purple_debug_misc(PLUGIN_ID, "history_window_open :: ER gc\n");
	return;
    }

    history_window_create();

    message = g_strdup_printf("<iq to='%s' id='xep135%x' type='get'><query xmlns='http://jabber.org/protocol/disco#info'/></iq>",
	    server, g_random_int());
    /*
    <iq to='debian6.sk' id='console3813359' type='get'>
	<query xmlns='http://jabber.org/protocol/disco#info'/>
    </iq>
    */
    message_send(message, gc);
    
    g_free(message);
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
    GtkWidget *toolbar_box, *hbox, *button;

    toolbar_box = gtkconv->toolbar;

    button = gtk_button_new_with_label("History");

    g_signal_connect(G_OBJECT(button), "clicked",
	    G_CALLBACK(history_window_open), (gpointer) gtkconv);

    hbox = gtk_hbox_new(FALSE, 5);
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
conv_created(PurpleConversation *conv, gpointer null)
{
    PidginConversation *gtkconv = PIDGIN_CONVERSATION(conv);

    g_return_if_fail(gtkconv != NULL);

    attach_to_gtkconv(gtkconv, NULL);
}

static gboolean
plugin_load(PurplePlugin *plugin)
{
    PurplePlugin *jabber;

    xep136 = plugin;
    attach_to_all_windows();

    purple_signal_connect(purple_conversations_get_handle(), "conversation-created", plugin, 
	    PURPLE_CALLBACK(conv_created), NULL);

    jabber = purple_find_prpl("prpl-jabber");
    if (!jabber)
	    return FALSE;

    purple_signal_connect(jabber, "jabber-receiving-xmlnode", xep136,
	    PURPLE_CALLBACK(xmlnode_received), NULL);

    return TRUE;
}

static gboolean
plugin_unload(PurplePlugin *plugin)
{
    if (history_window)
	gtk_widget_destroy(history_window->window);

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

    PLUGIN_ID,
    "XEP-136 plugin",
    "1.5",

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
