/* xep136_plugin.c */

#define PURPLE_PLUGINS
#define PLUGIN_ID "gtk-daniel_kraic-xep136_plugin" 

#include <string.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include <libpurple/version.h>
#include <libpurple/debug.h>

#include <pidgin/gtkimhtml.h>
#include <pidgin/gtkplugin.h>
#include "pidgin/gtkutils.h"

#include "xep136_plugin.h"

static PurplePlugin *xep136 = NULL;
static GList *list = NULL;
static char *xmlns_ejabberd = "http://www.xmpp.org/extensions/xep-0136.html#ns";
static char *xmlns_prosody = "urn:xmpp:archive";

/*--------------------------------------------------------------------------------
 * misc functions, increase_start_time, get_server_name, find_recipient  
 *--------------------------------------------------------------------------------*/

/* increase start time by one second */
static gchar *
increase_start_time(gchar *start)
{
    gchar *new = NULL;
    int i, c = 0;

    new = (gchar *) g_malloc0( strlen(start) * sizeof(gchar) );
    if (!new) {
	purple_debug_error(PLUGIN_ID, "ERROR: g_malloc0 new increase_start_time\n");
	return start;
    }

    for (i = 0; i < strlen(start); i++) {
	if (i == 18) {
	    c = (int) start[i];
	    c++;
	    new[i] = (char) c;
	} else {
	    new[i] = start[i];
	}
    }

    return new;
}

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
 * explore received xmlnode, manage collections  
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
	g_free(new);
	return;
    }

    message = g_strdup_printf("<iq id='%s' type='get'><retrieve xmlns='%s' with='%s' start='%s'><set xmlns='http://jabber.org/protocol/rsm'><max>10</max></set></retrieve></iq>", curr->id, curr->xmlns, new->with, new->start);

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
    gchar *last = NULL;

    if ( !xml->child ) {
	if ( !curr->coll ) {
	    //no collections received and no collections stored before
	    empty_collection(curr);
	    return;
	} else {
	    //no more collections to retrieve
	    return;
	}
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

    if (curr->end_tag_set)
	purple_debug_misc(PLUGIN_ID, "iq_list :: end_tag_set TRUE\n");
    else
	purple_debug_misc(PLUGIN_ID, "iq_list :: end_tag_set FALSE\n");

    if (!curr->end_tag_set) {
	//retrieve next 100
	last = increase_start_time(start);
	send_iq_list(curr, last, NULL);
    }
    
    //if (strcmp(last, start) != 0)
    if (last) 
	g_free(last);
}

static void
iq_pref(WindowStruct *curr, xmlnode *xml)
{
    xmlnode *c = NULL;
    xmlnode *d = NULL;
    gchar *text = NULL;

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

}

static void
iq_query_supported(WindowStruct *curr)
{
    gtk_widget_set_sensitive(curr->mainbox, TRUE);

    gtk_imhtml_clear(GTK_IMHTML(curr->imhtml));
    gtk_imhtml_append_text(GTK_IMHTML(curr->imhtml), "XEP-0136 supported", 0);
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

		curr->xmlns = xmlns_prosody;
		iq_query_supported(curr);

		return;

	    } else if (strcmp(( c->child)->data, xmlns_ejabberd) == 0) {

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

/*----------------------------------------------------------------------
 * send message, service discovery, show, enable, disable, status
 *----------------------------------------------------------------------*/
    
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
send_iq_list(WindowStruct *curr, gchar *from, gchar *to)
{
    PidginConversation *gtkconv = NULL;
    PurpleConversation *purple_conv = NULL;
    char *with = NULL;
    gchar *message = NULL;
    gchar *start = NULL;
    gchar *end = NULL;

    if (!curr->gtkconv) {
	purple_debug_misc(PLUGIN_ID, "send_iq_list :: ERR curr->gtkconv\n");
	return;
    }

    gtkconv = curr->gtkconv;
    if (!gtkconv) {
	purple_debug_misc(PLUGIN_ID, "send_iq_list :: ERR gtkconv\n");
	return;
    }

    purple_conv = gtkconv->active_conv;
    if (!purple_conv) {
	purple_debug_misc(PLUGIN_ID, "send_iq_list :: ERR purple_conv\n");
	return;
    }

    with = purple_conv->name;

    if (from)
	start = g_strdup_printf(" start='%s'", from);
    else
	start = g_strdup_printf(" ");

    if (to) {
	curr->end_tag_set = TRUE;
	end = g_strdup_printf(" end='%s'", to);
    } else {
	curr->end_tag_set = FALSE;
	end = g_strdup_printf(" ");
    }


    message = g_strdup_printf("<iq id='%s' type='get'><list xmlns='%s' with='%s'%s%s><set xmlns='http://jabber.org/protocol/rsm'><max>100</max></set></list></iq>", 
	    curr->id, curr->xmlns, with, start, end);

    message_send(message, curr->gtkconv);
    
    g_free(start);
    g_free(end);
    g_free(message);
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

    message = g_strdup_printf("<iq type='set' id='%s'><auto save='false' xmlns='%s'/></iq>", curr->id, curr->xmlns);

    message_send(message, curr->gtkconv);

    g_free(message);

    send_disco_info(curr);
}

static void
enable_clicked(GtkWidget *button, WindowStruct *curr)
{
    gchar *message = NULL;

    message = g_strdup_printf("<iq type='set' id='%s'><auto save='true' xmlns='%s'/></iq>", curr->id, curr->xmlns);

    message_send(message, curr->gtkconv);

    g_free(message);

    send_disco_info(curr);
}

static void
reset_clicked(GtkWidget *button, WindowStruct *curr)
{
    RightStruct *s = (RightStruct *) curr->showtable_struct;

    if (!s) {
	purple_debug_error(PLUGIN_ID, "ERROR: 's': reset_clicked\n");
	return;
    }

    gtk_combo_box_set_active(GTK_COMBO_BOX(s->from_year), -1);
    gtk_combo_box_set_active(GTK_COMBO_BOX(s->from_month), -1);
    gtk_combo_box_set_active(GTK_COMBO_BOX(s->from_day), -1);

    gtk_combo_box_set_active(GTK_COMBO_BOX(s->to_year), -1);
    gtk_combo_box_set_active(GTK_COMBO_BOX(s->to_month), -1);
    gtk_combo_box_set_active(GTK_COMBO_BOX(s->to_day), -1);
}

static gchar *
show_clicked_make_to(RightStruct *s)
{
    gchar *to = NULL;
    gchar *to_day = NULL;
    gchar *to_month = NULL;
    gchar *to_year = NULL;
    gint d, m, y;

    to_year = (gchar *) gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(s->to_year));
    to_month = (gchar *) gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(s->to_month));
    to_day = (gchar *) gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(s->to_day));

    if (!to_day || !to_month || !to_year) {
	//purple_debug_misc(PLUGIN_ID, "show_clicked_make_to :: !gchars\n");
	to = NULL;
    } else {
	//purple_debug_misc(PLUGIN_ID, "show_clicked_make_to :: atoi\n");
	d = atoi(to_day);
	m = atoi(to_month);
	y = atoi(to_year);
	
	to = g_strdup_printf("%02d-%02d-%02dT00:00:00Z", y, m ,d);
    }

    //purple_debug_misc(PLUGIN_ID, "show_clicked_make_to :: to %s\n", to);

    return to;
}

static gchar *
show_clicked_make_from(RightStruct *s)
{
    gchar *from = NULL;
    gchar *from_day = NULL;
    gchar *from_month = NULL;
    gchar *from_year = NULL;
    gint d, m, y;

    from_year = (gchar *) gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(s->from_year));
    from_month = (gchar *) gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(s->from_month));
    from_day = (gchar *) gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(s->from_day));

    if (!from_day || !from_month || !from_year) {
	//purple_debug_misc(PLUGIN_ID, "show_clicked_make_from :: !gchars\n");
	from = NULL;
    } else {
	//purple_debug_misc(PLUGIN_ID, "show_clicked_make_from :: atoi\n");
	d = atoi(from_day);
	m = atoi(from_month);
	y = atoi(from_year);
	
	from = g_strdup_printf("%02d-%02d-%02dT00:00:00Z", y, m ,d);
    }

    //purple_debug_misc(PLUGIN_ID, "show_clicked_make_from :: from %s\n", from);

    return from;
}

static void
show_clicked(GtkWidget *button, WindowStruct *curr)
{
    gchar *from = NULL;
    gchar *to = NULL;

    RightStruct *s = (RightStruct *) curr->showtable_struct;

    if (!s) {
	purple_debug_error(PLUGIN_ID, "ERROR: 's': show_clicked\n");
	return;
    }

    gtk_tree_store_clear(curr->treestore);

    if (curr->coll) {
	g_list_free(curr->coll);
	curr->coll = NULL;
    }

    from = show_clicked_make_from(s);
    to = show_clicked_make_to(s);

    purple_debug_misc(PLUGIN_ID, "SHOW_CLICKED :: from: %s to: %s\n", from, to);

    send_iq_list(curr, from, to);
    //send_iq_list(curr, NULL, NULL);
    
    if (from)
	g_free(from);

    if (to)
	g_free(to);
}

static void
send_disco_info(WindowStruct *curr)
{  
    gchar *server = get_server_name(curr->gtkconv);
    gchar *message = NULL;

    if (server == NULL) {
	purple_debug_error(PLUGIN_ID, "ERROR: 'server' send_disco_info \n");
	return;
    }

    message = g_strdup_printf("<iq to='%s' id='%s' type='get'><query xmlns='http://jabber.org/protocol/disco#info'/></iq>",
	    server, curr->id);

    message_send(message, curr->gtkconv);
    
    g_free(server);
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
create_right_table(WindowStruct *history_window)
{
    gint i;
    gchar *text = NULL;
    RightStruct *s = NULL;

    //create pointer to struct
    history_window->showtable_struct = g_malloc0(sizeof(RightStruct));
    s = (RightStruct *) history_window->showtable_struct;

    //create buttons
    s->enable = gtk_button_new_with_label("Enable");
    s->disable = gtk_button_new_with_label("Disable");
    s->status = gtk_button_new_with_label("Status");
    s->show_button = gtk_button_new_with_label("Show");
    s->reset_button = gtk_button_new_with_label("Reset");

    //buttons signals
    g_signal_connect(G_OBJECT(s->show_button), "clicked",
	    G_CALLBACK(show_clicked), (gpointer) history_window);

    g_signal_connect(G_OBJECT(s->reset_button), "clicked",
	    G_CALLBACK(reset_clicked), (gpointer) history_window);

    g_signal_connect(G_OBJECT(s->enable), "clicked",
	    G_CALLBACK(enable_clicked), (gpointer) history_window);

    g_signal_connect(G_OBJECT(s->disable), "clicked",
	    G_CALLBACK(disable_clicked), (gpointer) history_window);

    g_signal_connect(G_OBJECT(s->status), "clicked",
	    G_CALLBACK(status_clicked), (gpointer) history_window);

    //create labels
    s->label_from = gtk_label_new("From:");
    s->label_to = gtk_label_new("To:");
    s->label_enable = gtk_label_new("Enable archiving:");
    s->label_disable = gtk_label_new("Disable archiving:");
    s->label_status = gtk_label_new("Show status:");
    s->label_preferences = gtk_label_new("\n\nXEP-0136 preferences:");

    //create combo boxes
    s->from_day = gtk_combo_box_text_new();
    s->from_month = gtk_combo_box_text_new();
    s->from_year = gtk_combo_box_text_new();

    s->to_day = gtk_combo_box_text_new();
    s->to_month = gtk_combo_box_text_new();
    s->to_year = gtk_combo_box_text_new();

    for (i = 1; i <= 31; i++) {
	text = g_strdup_printf("%d", i);
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(s->from_day), (gchar *) text);
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(s->to_day), (gchar *) text);
	g_free(text);
    }

    for (i = 1; i <= 12; i++) {
	text = g_strdup_printf("%d", i);
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(s->from_month), (gchar *) text);
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(s->to_month), (gchar *) text);
	g_free(text);
    }

    for (i = 2000; i <= 2025; i++) {
	text = g_strdup_printf("%d", i);
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(s->from_year), (gchar *) text);
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(s->to_year), (gchar *) text);
	g_free(text);
    }

    //create table
    s->show_table = gtk_table_new(9, 3, FALSE);

    gtk_table_attach_defaults(GTK_TABLE(s->show_table), s->label_from,  0, 1, 0, 1);

    gtk_table_attach_defaults(GTK_TABLE(s->show_table), s->from_year,  0, 1, 1, 2);
    gtk_table_attach_defaults(GTK_TABLE(s->show_table), s->from_month, 1, 2, 1, 2);
    gtk_table_attach_defaults(GTK_TABLE(s->show_table), s->from_day,   2, 3, 1, 2);

    gtk_table_attach_defaults(GTK_TABLE(s->show_table), s->label_to,  0, 1, 2, 3);

    gtk_table_attach_defaults(GTK_TABLE(s->show_table), s->to_year,  0, 1, 3, 4);
    gtk_table_attach_defaults(GTK_TABLE(s->show_table), s->to_month, 1, 2, 3, 4);
    gtk_table_attach_defaults(GTK_TABLE(s->show_table), s->to_day,   2, 3, 3, 4);

    gtk_table_attach_defaults(GTK_TABLE(s->show_table), s->show_button, 1, 2, 4, 5);
    gtk_table_attach_defaults(GTK_TABLE(s->show_table), s->reset_button, 2, 3, 4, 5);

    gtk_table_attach_defaults(GTK_TABLE(s->show_table), s->label_preferences, 0, 3, 5, 6);

    gtk_table_attach_defaults(GTK_TABLE(s->show_table), s->label_enable, 0, 2, 6, 7);
    gtk_table_attach_defaults(GTK_TABLE(s->show_table), s->enable, 2, 3, 6, 7);

    gtk_table_attach_defaults(GTK_TABLE(s->show_table), s->label_disable, 0, 2, 7, 8);
    gtk_table_attach_defaults(GTK_TABLE(s->show_table), s->disable, 2, 3, 7, 8);

    gtk_table_attach_defaults(GTK_TABLE(s->show_table), s->label_status, 0, 2, 8, 9);
    gtk_table_attach_defaults(GTK_TABLE(s->show_table), s->status, 2, 3, 8, 9);

    gtk_table_set_col_spacings(GTK_TABLE(s->show_table), 5);
    gtk_table_set_row_spacings(GTK_TABLE(s->show_table), 5);
    gtk_container_set_border_width(GTK_CONTAINER(s->show_table), 5);
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
    gtk_window_set_default_size(GTK_WINDOW(history_window->window), 800, 350);

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
    create_right_table(history_window);

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
    gtk_box_pack_start(GTK_BOX(history_window->rightbox), (history_window->showtable_struct)->show_table, FALSE, FALSE, 0);
    //gtk_box_pack_start(GTK_BOX(history_window->rightbox), history_window->enable, FALSE, FALSE, 0);
    //gtk_box_pack_start(GTK_BOX(history_window->rightbox), history_window->disable, FALSE, FALSE, 0);
    //gtk_box_pack_start(GTK_BOX(history_window->rightbox), history_window->status, FALSE, FALSE, 0);

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
