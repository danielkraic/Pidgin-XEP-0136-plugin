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
    if (recipient->match == TRUE)
	return;

    purple_debug_misc(PLUGIN_ID, "find_recipient :: curr->id=%s recipient->id=%s\n", curr->id, recipient->id);

    if (strcmp(curr->id, recipient->id) == 0) {
	recipient->match = TRUE;
    	explore_xml(curr, recipient->xml);
    }
}

/*------------------------------------------------------------
 * receive and explore received xmlnode, iq_list, iq_query
 *------------------------------------------------------------*/

static void
send_propher_name(RetrieveCollection *coll, RetrieveCollection *new)
{
    if (strcmp(coll->start, new->start) == 0) {
	new->with = coll->with;
    }
}

static void
retrieve_collection(WindowStruct *curr, char *start)
{
    gchar *message = NULL;
    RetrieveCollection *new = NULL;
    PurpleConversation *purple_conv = (curr->gtkconv)->active_conv;
    
    if (!purple_conv) {
	purple_debug_error(PLUGIN_ID, "ERROR: 'purple_conv' retrieve_collection\n");
	return;
    }

    new = g_malloc0(sizeof(RetrieveCollection));
    if (!new) {
	purple_debug_error(PLUGIN_ID, "ERROR: g_malloc0 new RetrieveCollection\n");
	return;
    }

    new->start = start;
    new->with = NULL;

    g_list_foreach(curr->coll, (GFunc) send_propher_name, (gpointer) new);

    if (!new->with) {
	purple_debug_misc(PLUGIN_ID, "retrieve_collection :: !new->with\n");
	return;
    }

    message = g_strdup_printf("<iq id='%s' type='get'><retrieve xmlns='%s' with='%s' start='%s'><set xmlns='http://jabber.org/protocol/rsm'><max>100</max></set></retrieve></iq>", curr->id, curr->xmlns, new->with, new->start);

    message_send(message, curr->gtkconv);

    g_free(message);

    g_free(new);
}

static void
iq_retrieve_body(WindowStruct *curr, xmlnode *c, xmlnode *d)
{
    xmlnode *body = d->child;
    char *text = NULL;
    gchar *from_to = NULL;

    if (d->child) {
	if (body->data) {
	    text = xmlnode_get_data(d);

	    if (text) {
		from_to = g_strdup_printf("<b><font color='#ffcece'>%s</font></b>", c->name);

		gtk_imhtml_append_text(GTK_IMHTML(curr->imhtml), from_to, 0);
		gtk_imhtml_append_text(GTK_IMHTML(curr->imhtml), " :: ", 0);
		gtk_imhtml_append_text(GTK_IMHTML(curr->imhtml), text, 0);
		gtk_imhtml_append_text(GTK_IMHTML(curr->imhtml), "<br>", 0);

		g_free(text);
		g_free(from_to);
	    } else {
		purple_debug_misc(PLUGIN_ID, "iq_retrieve :: body :: data !get_data\n");
	    }
	}
    }
}

static void
iq_retrieve(WindowStruct *curr, xmlnode *xml)
{
    GtkTreeIter iter;
    xmlnode *c = NULL;
    xmlnode *d = NULL;
    char *data = NULL;

    gtk_imhtml_clear(GTK_IMHTML(curr->imhtml));

    for (c = xml->child; c; c = c->next) {
	if ( (strcmp(c->name, "from") == 0) || (strcmp(c->name, "to") == 0) ) { 
	    for (d = c->child; d; d = d->next) {
		if (strcmp(d->name, "body") == 0) { 
		    iq_retrieve_body(curr, c, d);

		}
	    }
	}
    }
}

static void
empty_collection(WindowStruct *curr)
{
    gtk_imhtml_clear(GTK_IMHTML(curr->imhtml));
    gtk_imhtml_append_text(GTK_IMHTML(curr->imhtml), "<b><font color='#ffcece'>Empty collection!</font></b><br>", 0);
}

static void
add_collection(WindowStruct *curr, gchar *start, gchar *with)
{
    RetrieveCollection *new = NULL;

    new = g_malloc0(sizeof(RetrieveCollection));
    if (!new) {
	purple_debug_error(PLUGIN_ID, "ERROR: g_malloc0 new RetrieveCollection\n");
	return;
    }

    new->start = g_strdup(start);
    new->with = g_strdup(with);

    //purple_debug_misc(PLUGIN_ID, "add_collection :: %s :: %s\n", new->with, new->start);
    
    if ( !(new->start) || !(new->with) ) {
	purple_debug_misc(PLUGIN_ID, "add_collection :: !prepend\n");
	return;
    }

    curr->coll = g_list_prepend(curr->coll, new);
}

static void
iq_list(WindowStruct *curr, xmlnode *xml)
{
    GtkTreeIter iter;
    xmlnode *c = NULL;
    xmlnode *d = NULL;
    char *with = NULL;
    char *start = NULL;

    gtk_tree_store_clear(curr->treestore);

    if (curr->coll) {
	g_list_free(curr->coll);
	curr->coll = NULL;
    }

    //if empty collection received and no collection retrieved before 
    if ( !(xml->child) && !(curr->coll) ) {
	empty_collection(curr);
	return;
    }

    for (c = xml->child; c; c = c->next) {
	if (strcmp(c->name, "chat") == 0) {
	    for (d = c->child; d; d = d->next) {

		if (strcmp(d->name, "start") == 0) {
		    if (!(d->data)) {
			purple_debug_error(PLUGIN_ID, "ERROR: iq_list :: start\n");
			return;
		    }
		    start = d->data;
		} else if (strcmp(d->name, "with") == 0) {
		    if (!(d->data)) {
			purple_debug_error(PLUGIN_ID, "ERROR: iq_list :: with\n");
			return;
		    }
		    with = d->data;
		}
	    }
	    
	    add_collection(curr, (gchar *) start, (gchar *) with);

	    gtk_tree_store_append(curr->treestore, &iter, NULL);
	    gtk_tree_store_set(curr->treestore, &iter, 0, start, -1);
	}
    }

    //retrieve next 100
    purple_debug_misc(PLUGIN_ID, "IQ_LIST :: houston, i needs more chats :-\(\n");
    //show_clicked(NULL, curr);
}

static void
iq_pref(WindowStruct *curr, xmlnode *xml)
{
    xmlnode *c = NULL;
    xmlnode *d = NULL;
    gchar *text = NULL;

    //purple_debug_misc(PLUGIN_ID, "iq_pref :: enter\n");

    for (c = xml->child; c; c = c->next) {
	if (strcmp(c->name, "auto") == 0) {
	    for (d = c->child; d; d = d->next) {
		if (strcmp(d->name, "save") == 0) {
		    text = g_strdup_printf("<b><font color='#ffcece'>auto save :: %s</font></b>", d->data);

		    if (!text) {
			purple_debug_error(PLUGIN_ID, "ERROR: 'iq_pref' !strdup(text)\n");
			return;
		    }

		    gtk_imhtml_append_text(GTK_IMHTML(curr->imhtml), text, 0);
		    gtk_imhtml_append_text(GTK_IMHTML(curr->imhtml), "<br>", 0);

		    g_free(text);
		}
	    }
	}
    }

    //purple_debug_misc(PLUGIN_ID, "iq_pref :: exit\n");
}

static void
iq_query_supported(WindowStruct *curr)
{
    gtk_widget_set_sensitive(curr->mainbox, TRUE);

    gtk_imhtml_clear(GTK_IMHTML(curr->imhtml));
    gtk_imhtml_append_text(GTK_IMHTML(curr->imhtml), "XEP-0136 supported", 0);
    gtk_imhtml_append_text(GTK_IMHTML(curr->imhtml), "<br>", 0);
    gtk_imhtml_append_text(GTK_IMHTML(curr->imhtml), "id = ", 0);
    gtk_imhtml_append_text(GTK_IMHTML(curr->imhtml), curr->id, 0);
    gtk_imhtml_append_text(GTK_IMHTML(curr->imhtml), "<br>", 0);

    send_pref_info(curr);
}

static void
iq_query(WindowStruct *curr, xmlnode *xml)
{
    xmlnode *c = NULL;
    gboolean feature = FALSE;

    for (c = xml->child; c; c = c->next) {
	if ( (strcmp(c->name, "feature") == 0) && (strcmp( (c->child)->name, "var" ) == 0) ) {
	    if (!feature)
		feature = TRUE;

	    if (strcmp(( c->child)->data, xmlns_prosody) == 0) {

		//purple_debug_misc(PLUGIN_ID, "disco :: PROSODY\n");
		curr->xmlns = xmlns_prosody;

		iq_query_supported(curr);

		return;

	    } else if (strcmp(( c->child)->data, xmlns_ejabberd) == 0) {

		//purple_debug_misc(PLUGIN_ID, "disco :: EJABBERD\n");
		curr->xmlns = xmlns_ejabberd;	

		iq_query_supported(curr);

		return;
	    }
	}
    }

    if (feature)
	purple_notify_warning(PLUGIN_ID, "XEP-0136 Not Supported!", "XEP-0136 Not Supported!",
	    "XEP-0136 Message Archiving Not Suppoted!");
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
	if (strcmp(c->name, "type") == 0) {
	    if (strcmp(c->data, "result") == 0) { 
		//purple_debug_misc(PLUGIN_ID, "explore_xml :: RESULT\n");
		break;
	    } else if (strcmp(c->data, "error") == 0) { 
		purple_debug_misc(PLUGIN_ID, "explore_xml :: xml type ERROR\n");
		return;
	    }
	}
    }

    for (c = xml->child; c; c = c->next) {
	if (strcmp(c->name, "query") == 0) {
	    purple_debug_misc(PLUGIN_ID, "EXPLORE_XML :: iq_query\n");
	    iq_query(curr, c);
	} else if (strcmp(c->name, "list") == 0) {
	    purple_debug_misc(PLUGIN_ID, "EXPLORE_XML :: iq_list\n");
	    iq_list(curr, c);
	} else if (strcmp(c->name, "chat") == 0) {
	    purple_debug_misc(PLUGIN_ID, "EXPLORE_XML :: iq_chat\n");
	    iq_retrieve(curr, c);
	} else if (strcmp(c->name, "pref") == 0) {
	    purple_debug_misc(PLUGIN_ID, "EXPLORE_XML :: iq_pref\n");
	    iq_pref(curr, c);
	}
    }
}

static void
xmlnode_received(PurpleConnection *gc, xmlnode **packet, gpointer null)
{
    xmlnode *xml = *packet;
    xmlnode *c = NULL;
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

    recipient->xml = xml;
    recipient->match = FALSE;

    if (strcmp(xml->name, "iq") == 0) {
	for (c = xml->child; c; c = c->next) {
	    if (strcmp(c->name, "id") == 0) {
		recipient->id = c->data;
		break;
	    }
	}

	purple_debug_misc(PLUGIN_ID, "xmlnode_received :: id=%s\n", recipient->id);

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
send_pref_info(WindowStruct *curr)
{
    gchar *message = NULL;

    message = g_strdup_printf("<iq id='%s' type='get'><pref xmlns='%s'/></iq>", curr->id, curr->xmlns);

    message_send(message, curr->gtkconv);

    g_free(message);
}

static void
status_clicked(GtkWidget *button, WindowStruct *curr)
{
    send_disco_info(curr);
}

static void
disable_clicked(GtkWidget *button, WindowStruct *curr)
{
    gchar *message = NULL;

    message = g_strdup_printf("<iq type='set' id='%s'><pref xmlns='%s'><auto save='false'/></pref></iq>",
	    curr->id, curr->xmlns);

    message_send(message, curr->gtkconv);

    g_free(message);

    send_disco_info(curr);
}

static void
enable_clicked(GtkWidget *button, WindowStruct *curr)
{
    gchar *message = NULL;

    message = g_strdup_printf("<iq type='set' id='%s'><pref xmlns='%s'><auto save='true'/></pref></iq>",
	    curr->id, curr->xmlns);

    message_send(message, curr->gtkconv);

    g_free(message);

    send_disco_info(curr);
}

static void
show_clicked(GtkWidget *button, WindowStruct *curr)
{
    PidginConversation *gtkconv = NULL;
    PurpleConversation *purple_conv = NULL;
    char *with = NULL;
    gchar *message = NULL;

    if (!curr->gtkconv) {
	purple_debug_misc(PLUGIN_ID, "SHOW_CLICKED :: ERR curr->gtkconv\n");
	return;
    }

    gtkconv = curr->gtkconv;
    if (!gtkconv) {
	purple_debug_misc(PLUGIN_ID, "SHOW_CLICKED :: ERR gtkconv\n");
	return;
    }

    purple_conv = gtkconv->active_conv;
    if (!purple_conv) {
	purple_debug_misc(PLUGIN_ID, "SHOW_CLICKED :: ERR purple_conv\n");
	return;
    }

    with = purple_conv->name;
    message = g_strdup_printf("<iq id='%s' type='get'><list xmlns='%s' with='%s'><set xmlns='http://jabber.org/protocol/rsm'><max>100</max></set></list></iq>", 
	    curr->id, curr->xmlns, with);

    message_send(message, curr->gtkconv);
    
    g_free(message);
}

static void
send_disco_info(WindowStruct *curr)
{  
    char *server = get_server_name(curr->gtkconv);
    gchar *message = NULL;

    if (server == NULL) {
	purple_debug_error(PLUGIN_ID, "ERROR: 'server' send_disco_info \n");
	return;
    }

    message = g_strdup_printf("<iq to='%s' id='%s' type='get'><query xmlns='http://jabber.org/protocol/disco#info'/></iq>",
	    server, curr->id);

    message_send(message, curr->gtkconv);
    
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
date_selected(GtkTreeSelection *sel, WindowStruct *curr)
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    gchar *date = NULL;

    if (!gtk_tree_selection_get_selected(sel, &model, &iter)) {
	purple_debug_misc(PLUGIN_ID, "date_selected :: !gtk_tree_selection_get_selected\n");
	return;
    }

    gtk_tree_model_get(model, &iter, 0, &date, -1);

    //purple_debug_misc(PLUGIN_ID, "date_selected :: %s\n", date);

    retrieve_collection(curr, date);

    g_free(date);
}

static void
create_left_list(WindowStruct *history_window)
{
    GtkCellRenderer *rend;
    GtkTreeViewColumn *col;
    GtkTreeSelection *sel;

    history_window->treestore = gtk_tree_store_new (1, G_TYPE_STRING);
    history_window->treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL (history_window->treestore));
    g_object_unref(G_OBJECT(history_window->treestore));

    rend = gtk_cell_renderer_text_new();
    col = gtk_tree_view_column_new_with_attributes ("Date", rend, "text", 0, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW(history_window->treeview), col);
    gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (history_window->treeview), FALSE);

    history_window->left = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(history_window->left),
	    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(history_window->left), history_window->treeview);

    sel = gtk_tree_view_get_selection(GTK_TREE_VIEW (history_window->treeview));
    gtk_tree_selection_set_mode(sel, GTK_SELECTION_SINGLE);
    g_signal_connect (G_OBJECT (sel), "changed",
	    G_CALLBACK (date_selected), (gpointer) history_window);
}

static void
history_window_create(WindowStruct *history_window)
{
    //history_window = g_new0(WindowStruct, 1);
    PidginConversation *gtkconv = history_window->gtkconv;
    PurpleConversation *conv = gtkconv->active_conv;

    history_window->window = pidgin_create_window("XEP-136 History", PIDGIN_HIG_BORDER, NULL, TRUE);
    gtk_window_set_default_size(GTK_WINDOW(history_window->window), 700, 350);

    g_signal_connect(G_OBJECT(history_window->window), "destroy", 
	    G_CALLBACK(history_window_destroy), (gpointer) history_window );

    //left
    create_left_list(history_window);

    //center
    history_window->label_username = gtk_label_new(conv->name);
    history_window->imhtml = gtk_imhtml_new(NULL, NULL);
    history_window->imhtml_win = gtk_scrolled_window_new(0, 0);
    //gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(history_window->imhtml_win), GTK_SHADOW_IN);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(history_window->imhtml_win), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    //right
    history_window->show = gtk_button_new_with_label("Show");
    history_window->enable = gtk_button_new_with_label("Enable");
    history_window->disable = gtk_button_new_with_label("Disable");
    history_window->status = gtk_button_new_with_label("Status");

    //buttons signals
    g_signal_connect(G_OBJECT(history_window->show), "clicked",
	    G_CALLBACK(show_clicked), (gpointer) history_window);

    g_signal_connect(G_OBJECT(history_window->enable), "clicked",
	    G_CALLBACK(enable_clicked), (gpointer) history_window);

    g_signal_connect(G_OBJECT(history_window->disable), "clicked",
	    G_CALLBACK(disable_clicked), (gpointer) history_window);

    g_signal_connect(G_OBJECT(history_window->status), "clicked",
	    G_CALLBACK(status_clicked), (gpointer) history_window);

    //boxing 
    history_window->mainbox = gtk_hbox_new(FALSE, 3);
    history_window->center= gtk_vbox_new(FALSE, 3);
    history_window->rightbox = gtk_vbox_new(FALSE, 3);

    //boxing center 
    gtk_container_add(GTK_CONTAINER(history_window->imhtml_win), history_window->imhtml);
    gtk_box_pack_start(GTK_BOX(history_window->center), history_window->label_username, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(history_window->center), history_window->imhtml_win, TRUE, TRUE, 0);
    /*gtk_box_pack_start(GTK_BOX(history_window->center), 
	    pidgin_make_scrollable(history_window->imhtml, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC, GTK_SHADOW_ETCHED_IN, -1, -1), 
	    TRUE, TRUE, 0);*/

    //boxing right
    gtk_box_pack_start(GTK_BOX(history_window->rightbox), history_window->show, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(history_window->rightbox), history_window->enable, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(history_window->rightbox), history_window->disable, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(history_window->rightbox), history_window->status, FALSE, FALSE, 0);

    //boxing main
    gtk_box_pack_start(GTK_BOX(history_window->mainbox), history_window->left, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(history_window->mainbox), history_window->center, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(history_window->mainbox), history_window->rightbox, FALSE, FALSE, 0);

    gtk_widget_set_sensitive(history_window->mainbox, FALSE);

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

    new->coll= NULL;
    new->gtkconv = gtkconv;
    new->id = g_strdup_printf("xep136%x", g_random_int());

    history_window_create(new);

    list = g_list_prepend(list, new);

    send_disco_info(new);
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

    //purple_debug_misc(PLUGIN_ID, "test included :: FALSE\n");

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
	//purple_debug_misc(PLUGIN_ID, "prpl-jabber check:: TRUE\n");
	return TRUE;
    } else {
	//purple_debug_misc(PLUGIN_ID, "prpl-jabber check:: FALSE\n");
	return FALSE;
    }
}

static void
destroy_windows(WindowStruct *curr)
{
    if (curr->coll)
	g_list_free(curr->coll);

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

    if (if_jabber(gtkconv))
	return;

    g_list_foreach(list, (GFunc) destroy_history_window, (gpointer) gtkconv);
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
    "0.2",

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
