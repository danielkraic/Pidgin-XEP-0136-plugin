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


/* 
 * --------------------------------------------------
 * skopirovane
 * --------------------------------------------------
 */

#if 0
#define BRACKET_COLOR "#940f8c"
#define TAG_COLOR "#8b1dab"
#define ATTR_NAME_COLOR "#a02961"
#define ATTR_VALUE_COLOR "#324aa4"
#define XMLNS_COLOR "#2cb12f"

static char *
xmlnode_to_pretty_str(xmlnode *node, int *len, int depth)
{
	GString *text = g_string_new("");
	xmlnode *c;
	char *node_name, *esc, *esc2, *tab = NULL;
	gboolean need_end = FALSE, pretty = TRUE;

	g_return_val_if_fail(node != NULL, NULL);

	/*
	if (pretty && depth) {
		tab = g_strnfill(depth, '\t');
		text = g_string_append(text, tab);
	}
	*/

	node_name = g_markup_escape_text(node->name, -1);
	g_string_append_printf(text,
	                       /*"<font color='" BRACKET_COLOR "'>&lt;</font>"*/
	                       "<font color='" TAG_COLOR "'><b>%s</b></font>",
	                       node_name);

	/*
	if (node->xmlns) {
		if ((!node->parent ||
		     !node->parent->xmlns ||
		     strcmp(node->xmlns, node->parent->xmlns)) &&
		    strcmp(node->xmlns, "jabber:client"))
		{
			char *xmlns = g_markup_escape_text(node->xmlns, -1);
			g_string_append_printf(text,
			                       " <font color='" ATTR_NAME_COLOR "'><b>xmlns</b></font>="
			                       "'<font color='" XMLNS_COLOR "'><b>%s</b></font>'",
			                       xmlns);
			g_free(xmlns);
		}
	}
	*/

	for (c = node->child; c; c = c->next)
	{
		if (c->type == XMLNODE_TYPE_ATTRIB) {
			esc = g_markup_escape_text(c->name, -1);
			esc2 = g_markup_escape_text(c->data, -1);
			/*
			g_string_append_printf(text,
			                       " <font color='" ATTR_NAME_COLOR "'><b>%s</b></font>="
			                       "'<font color='" ATTR_VALUE_COLOR "'>%s</font>'",
			                       esc, esc2);
					       */
			g_string_append_printf(text, " <b>%s</b>=%s", esc, esc2);
			g_free(esc);
			g_free(esc2);
		} else if (c->type == XMLNODE_TYPE_TAG || c->type == XMLNODE_TYPE_DATA) {
			if (c->type == XMLNODE_TYPE_DATA)
				pretty = FALSE;
			need_end = TRUE;
		}
	}

	if (need_end) {
	    /*
		g_string_append_printf(text,
		                       "<font color='"BRACKET_COLOR"'>&gt;</font>%s",
		                       "&gt; %s", pretty ? "<br>" : "");
				       */

		for (c = node->child; c; c = c->next)
		{
			if (c->type == XMLNODE_TYPE_TAG) {
				int esc_len;
				esc = xmlnode_to_pretty_str(c, &esc_len, depth+1);
				text = g_string_append_len(text, esc, esc_len);
				g_free(esc);
			} else if (c->type == XMLNODE_TYPE_DATA && c->data_sz > 0) {
				esc = g_markup_escape_text(c->data, c->data_sz);
				text = g_string_append(text, esc);
				g_free(esc);
			}
		}

		if(tab && pretty)
			text = g_string_append(text, tab);
		
		g_string_append_printf(text,
		                      /* "<font color='" BRACKET_COLOR "'>&lt;</font>/"*/
		                       "<font color='" TAG_COLOR "'><b>%s </b></font><br>",
		                      /*"<font color='" BRACKET_COLOR "'>&gt;</font><br>",*/
		                       node_name);
				       
	} else {
	    /*
		g_string_append_printf(text,
		                       "/<font color='" BRACKET_COLOR "'>&gt;</font><br>");
				       */
	}

	g_free(node_name);

	g_free(tab);

	if(len)
		*len = text->len;

	return g_string_free(text, FALSE);
}
#endif

static void
xmlnode_received_cb(PurpleConnection *gc, xmlnode **packet, gpointer null)
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
				
				gtk_imhtml_append_text(GTK_IMHTML(history_window->imhtml), "xep-0136 supported", 0);
				gtk_imhtml_append_text(GTK_IMHTML(history_window->imhtml), "<br>", 0);
			    }

			}
		    }
		}
	    }
	}

#if 0
	gtk_imhtml_append_text(GTK_IMHTML(history_window->imhtml), "  ", 0);
	gtk_imhtml_append_text(GTK_IMHTML(history_window->imhtml), (*packet)->name, 0);

	if ( (*packet)->xmlns != NULL ) {
	    gtk_imhtml_append_text(GTK_IMHTML(history_window->imhtml), " xmlns: ", 0);
	    gtk_imhtml_append_text(GTK_IMHTML(history_window->imhtml), (*packet)->xmlns, 0);
	}
	if ( (*packet)->data != NULL ) {
	    gtk_imhtml_append_text(GTK_IMHTML(history_window->imhtml), " data: ", 0);
	    gtk_imhtml_append_text(GTK_IMHTML(history_window->imhtml), (*packet)->data, 0);
	}
	gtk_imhtml_append_text(GTK_IMHTML(history_window->imhtml), "\n", 0);

#endif


#if 0
	char *str, *formatted;

	if (!strcmp((*packet)->name, "message")) {
	    /*
	    gtk_imhtml_append_text(GTK_IMHTML(history_window->imhtml), "<b>message</b><br>", 0);
	else
	    gtk_imhtml_append_text(GTK_IMHTML(history_window->imhtml), "<b>NIE message</b><br>", 0);
	    */

	    str = xmlnode_to_pretty_str(*packet, NULL, 0);
	    //formatted = g_strdup_printf("<body bgcolor='#ffcece'><pre>%s</pre></body>", str);
	    formatted = g_strdup_printf("<body><pre>%s</pre></body>", str);
	    gtk_imhtml_append_text(GTK_IMHTML(history_window->imhtml), formatted, 0);
	    g_free(formatted);
	    g_free(str);
	}

	/*
	<iq to='debian6.sk' id='consoleaf6052dd' type='get'>
	    <list xmlns='http://www.xmpp.org/extensions/xep-0136.html#ns' with='denko@debian6.sk' start='2008-01-01T00:00:00.000000Z'>
		<set xmlns='http://jabber.org/protocol/rsm'>
		    <max>100</max>
		</set>
	    </list>
	</iq>
	 */
#endif
}



/* 
 * --------------------------------------------------
 * skopirovane
 * --------------------------------------------------
 */



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

    gtk_container_add(GTK_CONTAINER(history_window->window), history_window->mainbox);
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
    /*
     
    <iq to='debian6.sk' id='console75f65bee' type='get'>
	<pref xmlns='http://www.xmpp.org/extensions/xep-0136.html#ns'/>
    </iq>

    */

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
    PurplePlugin *jabber;

    xep136 = plugin;
    attach_to_all_windows();

    purple_signal_connect(purple_conversations_get_handle(), "conversation-created", plugin, 
	    PURPLE_CALLBACK(conv_created_cb), NULL);

    jabber = purple_find_prpl("prpl-jabber");
    if (!jabber)
	    return FALSE;

    purple_signal_connect(jabber, "jabber-receiving-xmlnode", xep136,
	    PURPLE_CALLBACK(xmlnode_received_cb), NULL);

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
