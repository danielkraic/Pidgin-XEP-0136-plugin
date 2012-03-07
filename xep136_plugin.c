/* 
 * filename:	xep136_plugin.c 
 */

#define PURPLE_PLUGINS

#include "xep136_plugin.h"

PurplePlugin *xep136 = NULL; 	/* plugin id pointer */
GList *list = NULL;		/* list of pointers to WindowStruct items */

/* xmlns for ejabberd and prosody to xml messages */
char *xmlns_ejabberd = "http://www.xmpp.org/extensions/xep-0136.html#ns";
char *xmlns_prosody = "urn:xmpp:archive";

/*------------------------------------------------------------
 * plugin load and uload related functions
 *------------------------------------------------------------*/

/* check if conversation is jabber */
static gboolean
if_jabber(PidginConversation *gtkconv)
{
    PurpleConversation *conv = gtkconv->active_conv;
    PurpleAccount *acc = conv->account;
    char *jabber_id = "prpl-jabber";

    if(!acc) {
	purple_debug_misc(PLUGIN_ID, "prpl-jabber check:: ERROR acc\n");
	return FALSE;
    }

    // test jabber conversation
    if (strcmp(jabber_id, purple_account_get_protocol_id(acc)) == 0) {
	return TRUE;
    } else {
	return FALSE;
    }
}

static void
destroy_windows(WindowStruct *curr)
{
    if (curr->coll)
	g_list_free(curr->coll);

    g_free(curr->id);

    gtk_widget_destroy(curr->window);
}

static void
destroy_history_window(WindowStruct *curr, PidginConversation *gtkconv)
{
    if (curr->gtkconv == gtkconv) {
	destroy_windows(curr);
    }
}

static void
conv_deleted(PurpleConversation *conv, gpointer null)
{
    PidginConversation *gtkconv = NULL;

    if (!conv)
	purple_debug_error(PLUGIN_ID, "ERROR: 'conv_deleted': !conv\n");

    gtkconv = PIDGIN_CONVERSATION(conv);
    if (!gtkconv)
	purple_debug_error(PLUGIN_ID, "ERROR: 'conv_deleted': !gtkconv\n");

    g_return_if_fail(gtkconv != NULL);

    if (!list) {
	purple_debug_misc(PLUGIN_ID, "conv_deleted :: empty list\n");
	return;
    }

    if (!if_jabber(gtkconv))
	return;

    g_list_foreach(list, (GFunc) destroy_history_window, (gpointer) gtkconv);
}

static void
detach_from_gtkconv(PidginConversation *gtkconv, gpointer null)
{
    GtkWidget *toolbar_box, *hbox;

    if (!if_jabber(gtkconv))
	return;

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

    if (!if_jabber(gtkconv))
	return;

    toolbar_box = gtkconv->toolbar;

    button = gtk_button_new_with_label("History");

    g_signal_connect(G_OBJECT(button), "clicked",
	    G_CALLBACK(history_button_clicked), (gpointer) gtkconv);

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

/*------------------------------------------------------------
 * plugin load, unload, PurplePluginInfo, init_plugin
 *------------------------------------------------------------*/

static gboolean
plugin_load(PurplePlugin *plugin)
{
    PurplePlugin *jabber;

    xep136 = plugin;
    
    attach_to_all_windows();

    purple_signal_connect(purple_conversations_get_handle(), "conversation-created", plugin, 
	    PURPLE_CALLBACK(conv_created), NULL);

    purple_signal_connect(purple_conversations_get_handle(), "deleting-conversation", plugin, 
	    PURPLE_CALLBACK(conv_deleted), NULL);

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
    g_list_foreach(list, (GFunc) destroy_windows, NULL);

    g_list_free(list);
    list = NULL;

    detach_from_all_windows();

    return TRUE;
}

static PurplePluginInfo info = {
    PURPLE_PLUGIN_MAGIC,
    PURPLE_MAJOR_VERSION,
    PURPLE_MINOR_VERSION,
    PURPLE_PLUGIN_STANDARD,
    PIDGIN_PLUGIN_TYPE,
    0,
    NULL,
    PURPLE_PRIORITY_DEFAULT,

    PLUGIN_ID,
    "XEP-0136 plugin",
    "0.6",

    "XEP-0136 plugin",
    "Server Message Archiving",
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
