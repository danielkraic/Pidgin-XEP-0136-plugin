/*
 * xep136_receive.c
 */

#include "xep136_plugin.h"

static void send_propher_name(RetrieveCollection *coll, RetrieveCollection *new);
static void retrieve_collection_send_message(NewCollection *new, WindowStruct *curr);
static void retrieve_collection_find(RetrieveCollection *curr, RetrieveCollection *new);
static void iq_retrieve_body(WindowStruct *curr, xmlnode *c, xmlnode *d, gchar *secs, gchar *start);
static void iq_retrieve(WindowStruct *curr, xmlnode *xml);
static void iq_list(WindowStruct *curr, xmlnode *xml);
static void iq_pref(WindowStruct *curr, xmlnode *xml);
static void iq_query_supported(WindowStruct *curr);
static void iq_query(WindowStruct *curr, xmlnode *xml);

/*------------------------------------------------------------
 * functions related to receiving xml messages 
 *------------------------------------------------------------*/

void
print_r_collection(RetrieveCollection *curr)
{
    //g_list_foreach(curr->coll, (GFunc) print_r_collection, NULL);
    gchar *text;
    text = g_strdup_printf("#################### start %s with %s num %d\n", curr->start, curr->with, g_list_length(curr->to_retrieve));
    purple_debug_misc(PLUGIN_ID, text);
    g_free(text);
}

/* compare start time to set exact jid to xml message */
static void
send_propher_name(RetrieveCollection *coll, RetrieveCollection *new)
{
    if (strncmp(coll->start, new->start, 19) == 0) {
	//purple_debug_misc(PLUGIN_ID, " send_propher_name :: %s\n", coll->start);
	//purple_debug_misc(PLUGIN_ID, " send_propher_name :: %s\n", new->start);
	new->with = coll->with;
    }
}

/* retrieve collection: send message */
static void
retrieve_collection_send_message(NewCollection *new, WindowStruct *curr)
{
    gchar *message = NULL;
    
    message = g_strdup_printf("<iq id='%s' type='get'><retrieve xmlns='%s' with='%s' start='%s'><set xmlns='http://jabber.org/protocol/rsm'><max>100</max></set></retrieve></iq>", curr->id, curr->xmlns, new->with, new->start);

    message_send(message, curr->gtkconv);

    g_free(message);
}

/* retrieve collection: find collection for selected date */
static void
retrieve_collection_find(RetrieveCollection *curr, RetrieveCollection *new)
{
    /* exit if already found */
    if (new->start) 
	return;

    if ( strcmp(curr->date, new->date) == 0 ) {
	//purple_debug_misc(PLUGIN_ID, "retrieve_collection_find :: %s == %s\n", curr->date, new->date);

	new->start = curr->start;
	new->with = curr->with;
	new->to_retrieve = curr->to_retrieve;
    }
}

/* retrieve a collection */
void
retrieve_collection(WindowStruct *curr, gchar *date)
{
    RetrieveCollection *new = NULL;

    /* create new struct */
    new = (RetrieveCollection *) g_malloc0(sizeof(RetrieveCollection));
    if (!new) {
	purple_debug_error(PLUGIN_ID, "ERROR: g_malloc0 new RetrieveCollection\n");
	return;
    }

    new->date = date;
    new->start = NULL;

    /* find collection for selected date */
    g_list_foreach(curr->coll, (GFunc) retrieve_collection_find, (gpointer) new);

    if (!new->start) {
	purple_debug_misc(PLUGIN_ID, "add_collection :: !new->start\n");
	return;
    }

    /* set number of collections to show in imhtml */
    curr->number_of_convs_to_show = g_list_length(new->to_retrieve);

    purple_debug_misc(PLUGIN_ID, "#################### add_collection_create_new :: %s :: %s :: %s\n", new->date, new->start, new->with);
    purple_debug_misc(PLUGIN_ID, "#################### add_collection_create_new :: number %d number_of_convs_to_show\n", curr->number_of_convs_to_show);

    /* retieve all collections for selected date */
    g_list_foreach(new->to_retrieve, (GFunc) retrieve_collection_send_message, (gpointer) curr);

    g_free(new);
}

/* explore body of iq retrieve message */
static void
iq_retrieve_body(WindowStruct *curr, xmlnode *c, xmlnode *d, gchar *secs, gchar *start)
{
    xmlnode *body = d->child;
    char *text = NULL;
    gchar *from_to = NULL;
    gchar *imhtml_message = NULL;
    gchar *my_username = NULL;
    gchar *friends_username = NULL;

    if (!start) {
	purple_debug_error(PLUGIN_ID, "ERROR: !start iq_retrieve_body\n");
	return;
    }

    if (!secs) {
	purple_debug_error(PLUGIN_ID, "ERROR: !secs iq_retrieve_body\n");
	return;
    }

    my_username = (gchar *) get_my_username(curr->gtkconv);
    friends_username = (gchar *) get_friend_username(curr->gtkconv);

    if (d->child) {
	if (body->data) {
	    /* save data from body */
	    text = xmlnode_get_data(d);

	    /* create line to imhtml */
	    if (text) {
		if (strcmp(c->name, "from") == 0) {
		    //from_to = g_strdup_printf("<b><font color='#ff0000'>%s</font></b>", get_my_username(curr->gtkconv));
		    from_to = g_strdup_printf("<b><font color='#0000ff'>%s</font></b>", friends_username);
		    //from_to = g_strdup_printf("<b><font color='#ff0000'>%s</font></b>", "from");
		} else {
		    //from_to = g_strdup_printf("<b><font color='#0000ff'>%s</font></b>", get_friend_username(curr->gtkconv));
		    from_to = g_strdup_printf("<b><font color='#ff0000'>%s</font></b>", my_username);
		    //from_to = g_strdup_printf("<b><font color='#0000ff'>%s</font></b>", "to");
		}

	    	/* write to imhtml */
		//purple_debug_misc(PLUGIN_ID, "\nBODY %s + %s :: %s :: %s<br>\n", start, secs, from_to, text);
		//imhtml_message = g_strdup_printf("%s (%s + %s) :: %s<br>", from_to, start, secs, text);
		imhtml_message = g_strdup_printf("%s: %s<br>", from_to, text);

		if (!imhtml_message) {
		    purple_debug_error(PLUGIN_ID, "ERROR: !imhtml_message iq_retrieve_body\n");
		    return;
		}

		imhtml_text_save_message(curr, imhtml_message, secs, start);

		g_free(imhtml_message);
		g_free(text);
		g_free(from_to);
	    } else {
		purple_debug_misc(PLUGIN_ID, "iq_retrieve :: body :: data !get_data\n");
	    }
	}
    }

    g_free(my_username);
}

/* explore iq retrieve message */
static void
iq_retrieve(WindowStruct *curr, xmlnode *xml)
{
    GtkTreeIter iter;
    xmlnode *c = NULL;
    xmlnode *d = NULL;
    //char *data = NULL;
    gchar *secs = NULL;
    gchar *start = NULL;
    //gchar *debug_text = NULL;

    //gtk_imhtml_clear(GTK_IMHTML(curr->imhtml));

    /* to get start value */
    for (c = xml->child; c; c = c->next) {
	if (strcmp(c->name, "start") == 0) {
	    start = g_strdup(c->data);
	}
    }

    if (!start) {
	purple_debug_error(PLUGIN_ID, "ERROR: !start c->data iq_retrieve\n");
	return;
    }

    for (c = xml->child; c; c = c->next) {
	if ( (strcmp(c->name, "from") == 0) || (strcmp(c->name, "to") == 0) ) { 
	    for (d = c->child; d; d = d->next) {

		/* to ge secs value */
		if (strcmp(d->name, "secs") == 0) { 
		    secs = g_strdup(d->data);
		}

		if (strcmp(d->name, "body") == 0) { 
		    iq_retrieve_body(curr, c, d, secs, start);

		}
	    }
	}
    }

    g_free(start);
    g_free(secs);

    curr->number_of_convs_saved++;
    
    //debug_text = g_strdup_printf("<b>%d / %d</b><br>", curr->number_of_convs_saved, curr->number_of_convs_to_show);
    //gtk_imhtml_append_text(GTK_IMHTML(curr->imhtml), debug_text, 0);
    //g_free(debug_text);

    if (curr->number_of_convs_to_show == curr->number_of_convs_saved) {

	/* sort list and print conversations */
	curr->imhtml_list = g_list_sort(curr->imhtml_list, (GCompareFunc) imhtml_compare_func);
	g_list_foreach(curr->imhtml_list, (GFunc) show_imhtml_conv, (gpointer) curr);
	
	/* delete list and free memory */
	g_list_foreach(curr->imhtml_list, (GFunc) free_imhtml_item, NULL);
	g_list_free(curr->imhtml_list);
	curr->imhtml_list = NULL;

	curr->number_of_convs_to_show = 0;
	curr->number_of_convs_saved = 0;
	//gtk_imhtml_append_text(GTK_IMHTML(curr->imhtml), "<b>juchu juchu</b><br>", 0);
    }
}

/* explore iq list message */
static void
iq_list(WindowStruct *curr, xmlnode *xml)
{
    xmlnode *c = NULL;
    xmlnode *d = NULL;
    char *with = NULL;
    char *start = NULL;
    gchar *last = NULL;
    gchar *text;

    /* handle empty collection */
    if ( !xml->child ) {
	if ( !curr->coll ) {
	    /* no collection received and no collection stored before */
	    empty_collection(curr);
	    return;
	} else {
	    /* no more collections to retrieve */
	    return;
	}
    }

    /* find "start" and "date" */
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

	    /* save collection with raw date */
	    text = g_strdup_printf("############### add_collection :: start %s :: with %s\n", start, with);
	    purple_debug_misc(PLUGIN_ID, text);

	    add_collection(curr, (gchar *) start, (gchar *) with);

	    g_free(text);
	}
    }

    /*
    if (curr->end_tag_set)
	purple_debug_misc(PLUGIN_ID, "iq_list :: end_tag_set TRUE\n");
    else
	purple_debug_misc(PLUGIN_ID, "iq_list :: end_tag_set FALSE\n");
    */

    /* if end tag is not set, retrieve next 100 collections */
    /*
    if (!curr->end_tag_set) {
	last = increase_start_time(start);
	send_iq_list(curr, last, NULL);
    }
    */
    
    if (last) 
	g_free(last);
}

/* explore iq preferences massage */
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

		    /* write auto save status to imhtml */
		    text = g_strdup_printf("<b><font color='#cc0000'>auto save :: %s</font></b>", d->data);

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

/* write "xep-0136 supported" message to imhtml */
static void
iq_query_supported(WindowStruct *curr)
{
    gtk_widget_set_sensitive(curr->mainbox, TRUE);

    gtk_imhtml_clear(GTK_IMHTML(curr->imhtml));
    gtk_imhtml_append_text(GTK_IMHTML(curr->imhtml), "<b>XEP-0136 supported!</b><br />", 0);

    /* send message to determine preferences */
    send_pref_info(curr);
}

/* explore iq query message */
static void
iq_query(WindowStruct *curr, xmlnode *xml)
{
    xmlnode *c = NULL;
    gboolean feature = FALSE;
    gchar *error_text = NULL;
    gchar *server_name = NULL;

    for (c = xml->child; c; c = c->next) {
	if ( (strcmp(c->name, "feature") == 0) && (strcmp( (c->child)->name, "var" ) == 0) ) {
	    if (!feature)
		feature = TRUE;

	    /* find xep-0139 support with xmlns_prosody */
	    if (strncmp(( c->child)->data, xmlns_prosody, strlen(xmlns_prosody)) == 0) {

		purple_debug_misc(PLUGIN_ID, "iq_query :: xmlns_prosody\n");
		//purple_notify_warning(PLUGIN_ID, "XEP-0136 Not Supported!", (c->child)->data, (c->child)->data);

		curr->xmlns = xmlns_prosody;
		iq_query_supported(curr);

		return;

	    /* find xep-0139 support with xmlns_ejabberd */
	    } else if (strcmp(( c->child)->data, xmlns_ejabberd) == 0) {

		curr->xmlns = xmlns_ejabberd;	
		iq_query_supported(curr);

		return;
	    }
	}
    }

    /* warning dialog with "not supported" message */
    if (feature) {
	server_name = (gchar *) get_server_name(curr->gtkconv);
	//error_text = g_strdup_printf("XEP-0136 Message Archiving for %s Not Supported!", get_server_name(curr->gtkconv));
	error_text = g_strdup_printf("XEP-0136 Message Archiving for %s Not Supported!", server_name);

	purple_notify_warning(PLUGIN_ID, "XEP-0136 Not Supported!", "XEP-0136 Not Supported!", error_text);

	g_free(error_text);
	g_free(server_name);
    }
}

/* explore received xml message */
void
explore_xml(WindowStruct *curr, xmlnode *xml)
{
    xmlnode *c = NULL;

    if (!curr || !xml) {
	purple_debug_error(PLUGIN_ID, "ERROR: 'explore_xml': !curr || !xlm\n");
	return;
    }

    /* check for error */
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

    /* determine message type */
    for (c = xml->child; c; c = c->next) {
	if (strcmp(c->name, "query") == 0) {
	    //purple_debug_misc(PLUGIN_ID, "EXPLORE_XML :: iq_query\n");
	    iq_query(curr, c);
	} else if (strcmp(c->name, "list") == 0) {
	    //purple_debug_misc(PLUGIN_ID, "EXPLORE_XML :: iq_list\n");
	    iq_list(curr, c);
	} else if (strcmp(c->name, "chat") == 0) {
	    //purple_debug_misc(PLUGIN_ID, "EXPLORE_XML :: iq_chat\n");
	    iq_retrieve(curr, c);
	} else if (strcmp(c->name, "pref") == 0) {
	    //purple_debug_misc(PLUGIN_ID, "EXPLORE_XML :: iq_pref\n");
	    iq_pref(curr, c);
	}
    }
}

/* handle received xml message */
void
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

    /* find recipient with xmpp message id */
    if (strcmp(xml->name, "iq") == 0) {
	for (c = xml->child; c; c = c->next) {
	    if (strcmp(c->name, "id") == 0) {
		recipient->id = c->data;
		break;
	    }
	}

	//purple_debug_misc(PLUGIN_ID, "xmlnode_received :: id=%s\n", recipient->id);

	g_list_foreach(list, (GFunc) find_recipient, (gpointer) recipient);
    }

    g_free(recipient);
}
