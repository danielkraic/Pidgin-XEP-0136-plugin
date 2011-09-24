/* xep136_plugin.c */

#define PURPLE_PLUGINS
#define PLUGIN_ID "gtk-daniel_kraic-xep136_plugin" 

#include <string.h>
#include <gtk/gtk.h>
#include <version.h>
#include <gtkimhtml.h>
#include <gtkplugin.h>
#include <debug.h>
#include "gtkutils.h"

#include "xep136_plugin.h"

static PurplePlugin *xep136 = NULL;
static GList *list = NULL;
static char *xmlns_ejabberd = "http://www.xmpp.org/extensions/xep-0136.html#ns";
static char *xmlns_prosody = "urn:xmpp:archive";

/*--------------------------------------------------------------------------------
 * misc functions, get_server_name, find_recipient
 *--------------------------------------------------------------------------------*/

/*
static char *
get_my_username(WindowStruct *curr)
{
    PidginConversation *gtkconv = curr->gtkconv;
    PurpleConversation *purple_conv = gtkconv->active_conv;
    PurpleAccount *acc = purple_conv->account;
    char *my_username = NULL; 

    if (!acc) {
	purple_debug_error(PLUGIN_ID, "ERROR: 'get_my_username': !acc\n");
	return;
    }

    my_username = acc->username;

    return my_username;
}
*/

static gchar * 
get_server_name(PidginConversation *gtkconv)
{
    PurpleConversation *purple_conv = gtkconv->active_conv;
    PurpleAccount *acc = purple_conv->account;
    char *username= acc->username;

    gchar *server = NULL;
    gchar *zav = NULL; //pointer to '@'
    gchar *lom = NULL; //pointer to '/'

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
find_recipient(WindowStruct *curr, Recipient_info *recipient)
{
    PidginConversation *gtkconv = NULL;
    PurpleConversation *purple_conv = NULL;
    PurpleAccount *acc = NULL;
    PurpleConnection *gc = NULL;

    if (recipient->match == TRUE)
	return;

    gtkconv = curr->gtkconv;
    if (!gtkconv) {
	purple_debug_misc(PLUGIN_ID, "FIND_RECIPIENT :: ERR gtkconv\n");
	return;
    }

    purple_conv = gtkconv->active_conv;
    if (!purple_conv) {
	purple_debug_misc(PLUGIN_ID, "FIND_RECIPIENT :: ERR purple_conv\n");
	return;
    }

    acc = purple_conv->account;
    if (!acc) {
	purple_debug_misc(PLUGIN_ID, "FIND_RECIPIENT :: ERR acc\n");
	return;
    }

    gc = acc->gc;
    if (!gc) {
	purple_debug_misc(PLUGIN_ID, "FIND_RECIPIENT :: ERR gc\n");
	return;
    }

    if (gc == recipient->gc) {
	//purple_debug_misc(PLUGIN_ID, "FIND_RECIPIENT :: gc match for %s\n", get_my_username(curr));
	recipient->match = TRUE;
    	explore_xml(curr, recipient->xml);
    }
}

/*------------------------------------------------------------
 * receive and explore received xmlnode, iq_list, iq_query
 *------------------------------------------------------------*/

/*
static void
retrieve_collection(WindowStruct *curr, char *start)
//show_clicked(GtkWidget *button, PidginConversation *gtkconv)
{
    gchar *message = NULL;

    PurpleConversation *purple_conv = (curr->gtkconv)->active_conv;
    //char *with = "denko@debian6.sk";
    char *with = purple_conv->name;
    
    if (!purple_conv) {
	purple_debug_error(PLUGIN_ID, "ERROR: 'purple_conv' retrieve_collection\n");
	return;
    }
    
    message = g_strdup_printf("<iq id='xep135%x' type='get'><retrieve xmlns='%s' with='%s' start='%s'><set xmlns='http://jabber.org/protocol/rsm'><max>100</max></set></retrieve></iq>", g_random_int(), curr->xmlns, with, start);

    //purple_debug_misc(PLUGIN_ID, "history_window_open :: %s\n", message);
    message_send(message, curr->gtkconv);
    
    g_free(message);
}
*/

static void
iq_list(WindowStruct *curr, xmlnode *xml)
{
    xmlnode *c = NULL;
    xmlnode *d = NULL;

    for (c = xml->child; c; c = c->next) {
	if (strcmp(c->name, "chat") == 0) {
	    for (d = c->child; d; d = d->next) {
		if (strcmp(d->name, "start") == 0) {
		    if (!(d->data)) {
			purple_debug_error(PLUGIN_ID, "ERROR: iq_list :: d->data\n");
			return;
		    }
		    //purple_debug_misc(PLUGIN_ID, "retrieve_collection :: %s\n", d->data);
		    //retrieve_collection(curr, d->data);
		    
		    gtk_imhtml_append_text(GTK_IMHTML(curr->imhtml), d->data, 0);
		    gtk_imhtml_append_text(GTK_IMHTML(curr->imhtml), "<br>", 0);
		}
	    }
	}
    }
}

static void
iq_query(WindowStruct *curr, xmlnode *xml)
{
    xmlnode *c = NULL;

    for (c = xml->child; c; c = c->next) {
	if ( (strcmp(c->name, "feature") == 0) && (strcmp( (c->child)->name, "var" ) == 0) ) {
	    if (strcmp(( c->child)->data, xmlns_prosody) == 0) {

		purple_debug_misc(PLUGIN_ID, "disco :: PROSODY\n");
		curr->xmlns = xmlns_prosody;

		gtk_widget_set_sensitive(curr->rightbox, TRUE);
		gtk_imhtml_append_text(GTK_IMHTML(curr->imhtml), "XEP-0136 supported!", 0);
		gtk_imhtml_append_text(GTK_IMHTML(curr->imhtml), "<br>", 0);

		return;

	    } else if (strcmp(( c->child)->data, xmlns_ejabberd) == 0) {

		purple_debug_misc(PLUGIN_ID, "disco :: EJABBERD\n");
		curr->xmlns = xmlns_ejabberd;	

		gtk_widget_set_sensitive(curr->rightbox, TRUE);
		gtk_imhtml_append_text(GTK_IMHTML(curr->imhtml), "XEP-0136 supported!", 0);
		gtk_imhtml_append_text(GTK_IMHTML(curr->imhtml), "<br>", 0);

		return;
	    }
	}
    }
}

static void
explore_xml(WindowStruct *curr, xmlnode *xml)
{
    xmlnode *c = NULL;

    if (!curr || !xml) {
	purple_debug_error(PLUGIN_ID, "ERROR: 'explore_xml': !curr || !xlm\n");
	return;
    }

    for (c = xml->child; c; c = c->next) {

	if (strcmp(c->name, "query") == 0) {
	    purple_debug_misc(PLUGIN_ID, "EXPLORE_XML :: iq_query\n");
	    iq_query(curr, c);
	} else if (strcmp(c->name, "list") == 0) {
	    purple_debug_misc(PLUGIN_ID, "EXPLORE_XML :: iq_list\n");
	    iq_list(curr, c);
	}
    }
}


static void
xmlnode_received(PurpleConnection *gc, xmlnode **packet, gpointer null)
{
    xmlnode *xml = *packet;
    Recipient_info *recipient = NULL; 

    if (list == NULL) {
	purple_debug_misc(PLUGIN_ID, "XMLNODE_RECEIVED :: empty list\n");
	return;
    }

    recipient = g_malloc0(sizeof(Recipient_info));

    if (!recipient) {
	purple_debug_error(PLUGIN_ID, "ERROR: g_malloc0 new Recipient_info\n");
	return;
    }

    recipient->gc = gc;
    recipient->xml = xml;
    recipient->match = FALSE;

    if (strcmp(xml->name, "iq") == 0) {
	//purple_debug_misc(PLUGIN_ID, "XMLNODE_RECEIVED :: received iq\n");
	g_list_foreach(list, (GFunc) find_recipient, (gpointer) recipient);
    }

    g_free(recipient);
}

/*------------------------------------------------------------
 * send message, service discovery, list collections
 *------------------------------------------------------------*/
    
static void
message_send(char *message, PidginConversation *gtkconv)
{
    PurpleConversation *purple_conv = gtkconv->active_conv;
    PurpleAccount *acc = purple_conv->account;
    PurpleConnection *gc = acc->gc;

    PurplePluginProtocolInfo *prpl_info = NULL;

    if (!gc) {
	purple_debug_error(PLUGIN_ID, "ERROR: 'gc' message_send\n");
	return;
    }

    if (gc)
	    prpl_info = PURPLE_PLUGIN_PROTOCOL_INFO(gc->prpl);

    if (prpl_info && prpl_info->send_raw != NULL)
	    prpl_info->send_raw(gc, message, strlen(message));
}

static void
show_clicked(GtkWidget *button, WindowStruct *curr)
{
    PidginConversation *gtkconv = curr->gtkconv;
    PurpleConversation *purple_conv = gtkconv->active_conv;
    char *with = purple_conv->name;
    gchar *message = NULL;
    
    message = g_strdup_printf("<iq id='xep135%x' type='get'><list xmlns='%s' with='%s'><set xmlns='http://jabber.org/protocol/rsm'><max>100</max></set></list></iq>", 
	    g_random_int(), curr->xmlns, with);

    //purple_debug_misc(PLUGIN_ID, "history_window_open :: %s\n", message);
    message_send(message, curr->gtkconv);
    
    g_free(message);

}

static void
send_disco_info(PidginConversation *gtkconv)
{  
    char *server = get_server_name(gtkconv);
    gchar *message = NULL;

    if (server == NULL) {
	purple_debug_error(PLUGIN_ID, "ERROR: 'server' send_disco_info \n");
	return;
    }

    message = g_strdup_printf("<iq to='%s' id='xep135%x' type='get'><query xmlns='http://jabber.org/protocol/disco#info'/></iq>",
	    server, g_random_int());

    message_send(message, gtkconv);
    
    g_free(message);

}

/*------------------------------------------------------------
 * GTK create, destroy, history window
 *------------------------------------------------------------*/

static void
history_window_destroy(GtkWidget *window, WindowStruct *curr)
{
    list = g_list_remove(list, curr);
    g_free(curr);
}

static void
history_window_create(WindowStruct *history_window)
{
    //history_window = g_new0(WindowStruct, 1);
    PidginConversation *gtkconv = history_window->gtkconv;
    PurpleConversation *conv = gtkconv->active_conv;

    history_window->window = pidgin_create_window("XEP-136 History", PIDGIN_HIG_BORDER, NULL, TRUE);
    gtk_window_set_default_size(GTK_WINDOW(history_window->window), 400, 350);

    g_signal_connect(G_OBJECT(history_window->window), "destroy", 
	    G_CALLBACK(history_window_destroy), (gpointer) history_window );

    history_window->imhtml = gtk_imhtml_new(NULL, NULL);
    history_window->label_username = gtk_label_new(conv->name);
    history_window->label = gtk_label_new("Search ");
    history_window->entry = gtk_entry_new();
    history_window->button = gtk_button_new_from_stock(GTK_STOCK_FIND);

    history_window->show = gtk_button_new_with_label("Show");
    history_window->next = gtk_button_new_with_label("Next");
    history_window->enable = gtk_button_new_with_label("Enable");
    history_window->disable = gtk_button_new_with_label("Disable");

    g_signal_connect(G_OBJECT(history_window->show), "clicked",
	    G_CALLBACK(show_clicked), (gpointer) history_window);

    history_window->mainbox = gtk_hbox_new(FALSE, 3);
    history_window->rightbox = gtk_vbox_new(FALSE, 3);

    history_window->hbox = gtk_hbox_new(FALSE, 3);
    history_window->vbox = gtk_vbox_new(FALSE, 3);

    gtk_box_pack_start(GTK_BOX(history_window->mainbox), history_window->vbox, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(history_window->mainbox), history_window->rightbox, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(history_window->vbox), history_window->label_username, FALSE, FALSE, 0);
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
history_window_open(PidginConversation *gtkconv)
{
    WindowStruct *new = g_malloc0(sizeof(WindowStruct));

    if (!new) {
	purple_debug_error(PLUGIN_ID, "ERROR: g_malloc0 new WindowStruct\n");
	return;
    }

    new->gtkconv = gtkconv;
    history_window_create(new);

    list = g_list_prepend(list, new);

    send_disco_info(gtkconv);
}

static void
history_window_exist_test(WindowStruct *curr, Test_struct *test)
{
    if (curr->gtkconv == test->gtkconv)
	test->included = TRUE;
}

static void
history_button_clicked(GtkWidget *button, PidginConversation *gtkconv)
{
    Test_struct *test = g_malloc0(sizeof(Test_struct));

    test->gtkconv = gtkconv;
    test->included = FALSE;

    g_list_foreach(list, (GFunc) history_window_exist_test, (gpointer) test);

    if (test->included == TRUE) {
	g_free(test);
	purple_debug_misc(PLUGIN_ID, "test included :: TRUE\n");
	return;
    }

    g_free(test);

    purple_debug_misc(PLUGIN_ID, "test included :: FALSE\n");

    history_window_open(gtkconv);
}

/*------------------------------------------------------------
 * attach, detach, history_button
 *------------------------------------------------------------*/

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
    if (g_strcmp0(jabber_id, purple_account_get_protocol_id(acc))) {
	purple_debug_misc(PLUGIN_ID, "prpl-jabber check:: TRUE\n");
	return TRUE;
    } else {
	purple_debug_misc(PLUGIN_ID, "prpl-jabber check:: FALSE\n");
	return FALSE;
    }
}

static void
detach_from_gtkconv(PidginConversation *gtkconv, gpointer null)
{
    GtkWidget *toolbar_box, *hbox;

    if (if_jabber(gtkconv))
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

    if (if_jabber(gtkconv))
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

static void
destroy_windows(WindowStruct *curr)
{
    gtk_widget_destroy(curr->window);
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
    "0.1",

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
