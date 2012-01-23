/*
 * xep136_send.c
 */

#include "xep136_plugin.h"

static gchar * show_clicked_make_to(RightStruct *s);
static gchar * show_clicked_make_from(RightStruct *s);
static void free_colls_to_retrieve(NewCollection *new);
static void free_colls_item(RetrieveCollection *coll);

/*----------------------------------------------------------------------
 * functions related to sending xml messages
 *----------------------------------------------------------------------*/

/* send raw xml message */
void
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

/* request the first page of list with same jid */
void
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

    /* making message: with, start, end */
    with = purple_conv->name;

    if (from)
	start = g_strdup_printf(" start='%s'", from);
    else
	start = g_strdup_printf(" ");

    if (to) {
	/* set end tag */
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

/* request server preferences */
void
send_pref_info(WindowStruct *curr)
{
    gchar *message = NULL;

    message = g_strdup_printf("<iq id='%s' type='get'><pref xmlns='%s'/></iq>", curr->id, curr->xmlns);

    message_send(message, curr->gtkconv);

    g_free(message);
}

/* handle status button */
void
status_clicked(GtkWidget *button, WindowStruct *curr)
{
    g_list_foreach(curr->coll, (GFunc) print_r_collection, NULL);
    send_disco_info(curr);
}

/* handle disable button: set auto save FALSE */
void
disable_clicked(GtkWidget *button, WindowStruct *curr)
{
    gchar *message = NULL;

    message = g_strdup_printf("<iq type='set' id='%s'><auto save='false' xmlns='%s'/></iq>", curr->id, curr->xmlns);

    message_send(message, curr->gtkconv);

    g_free(message);

    send_disco_info(curr);
}

/* handle enable button: set auto save TRUE */
void
enable_clicked(GtkWidget *button, WindowStruct *curr)
{
    gchar *message = NULL;

    message = g_strdup_printf("<iq type='set' id='%s'><auto save='true' xmlns='%s'/></iq>", curr->id, curr->xmlns);

    message_send(message, curr->gtkconv);

    g_free(message);

    send_disco_info(curr);
}

/* handle reset button: reset combo boxes */
void
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

/* return "to" as gchar from gtk_combo_box */
static gchar *
show_clicked_make_to(RightStruct *s)
{
    gchar *to = NULL;
    gchar *to_day = NULL;
    gchar *to_month = NULL;
    gchar *to_year = NULL;
    gint d, m, y;

    to_year = (gchar *) gtk_combo_box_get_active_text(GTK_COMBO_BOX(s->to_year));
    to_month = (gchar *) gtk_combo_box_get_active_text(GTK_COMBO_BOX(s->to_month));
    to_day = (gchar *) gtk_combo_box_get_active_text(GTK_COMBO_BOX(s->to_day));

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

/* return "from" as gchar from gtk_combo_box */
static gchar *
show_clicked_make_from(RightStruct *s)
{
    gchar *from = NULL;
    gchar *from_day = NULL;
    gchar *from_month = NULL;
    gchar *from_year = NULL;
    gint d, m, y;

    from_year = (gchar *) gtk_combo_box_get_active_text(GTK_COMBO_BOX(s->from_year));
    from_month = (gchar *) gtk_combo_box_get_active_text(GTK_COMBO_BOX(s->from_month));
    from_day = (gchar *) gtk_combo_box_get_active_text(GTK_COMBO_BOX(s->from_day));

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

/* free item of to_retrieve list */
static void
free_colls_to_retrieve(NewCollection *new)
{
    if (!new) { 
	purple_debug_misc(PLUGIN_ID, "free_colls_item :: !new\n");
	return;
    }

    if (new->date) g_free(new->date);
    if (new->start) g_free(new->start);
    if (new->with)  g_free(new->with);
}

/* free item of curr->coll list */
static void
free_colls_item(RetrieveCollection *coll)
{
    if (!coll) {
	purple_debug_misc(PLUGIN_ID, "free_colls_item :: !coll\n");
	return;
    }

    if (coll->to_retrieve) {
	//g_list_foreach(coll->to_retrieve, (GFunc) free_colls_to_retrieve, NULL);
	g_list_free(coll->to_retrieve);
	coll->to_retrieve = NULL;
    }

    if (coll->date) g_free(coll->date);
    if (coll->start) g_free(coll->start);
    if (coll->with) g_free(coll->with);
}

/* handle show button: send request with "start" = from  and "end" = to */
void
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

    /*
    if (curr->coll) {
	g_list_free(curr->coll);
	curr->coll = NULL;
    }
    */

    /* free previous stored curr->coll list */
    if (curr->coll) {
	g_list_foreach(curr->coll, (GFunc) free_colls_item, NULL);
	g_list_free(curr->coll);
	curr->coll = NULL;
    }

    from = show_clicked_make_from(s);
    to = show_clicked_make_to(s);

    //purple_debug_misc(PLUGIN_ID, "SHOW_CLICKED :: from: %s to: %s\n", from, to);

    send_iq_list(curr, from, to);
    
    if (from)
	g_free(from);

    if (to)
	g_free(to);
}

/* send client service discovery request */
void
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
