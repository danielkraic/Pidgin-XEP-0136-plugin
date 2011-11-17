/* 
 * filename:	xep136_plugin.h 
 *
 * author: 	Daniel Kraic
 * email:	danielkraic@gmail.com
 * date:	2011-11-16
 * version:	v0.4
 *
 */

#ifndef _XEP136_H
#define _XEP136_H

#define PLUGIN_ID "gtk-daniel_kraic-xep136_plugin" 

#include <string.h>
#include <stdlib.h>
#include <time.h>

#include <gtk/gtk.h>

#include <libpurple/version.h>
#include <libpurple/debug.h>

#include <pidgin/gtkimhtml.h>
#include <pidgin/gtkplugin.h>
#include "pidgin/gtkutils.h"

typedef struct {
    GtkWidget *show_table;

    GtkWidget *label_preferences;
    GtkWidget *label_enable;
    GtkWidget *label_disable;
    GtkWidget *label_status;
    GtkWidget *enable;
    GtkWidget *disable;
    GtkWidget *status;

    GtkWidget *label_from;
    GtkWidget *from_day;
    GtkWidget *from_month;
    GtkWidget *from_year;

    GtkWidget *label_to;
    GtkWidget *to_day;
    GtkWidget *to_month;
    GtkWidget *to_year;

    GtkWidget *show_button;
    GtkWidget *reset_button;

} RightStruct;

typedef struct _WindowStruct {
    GtkWidget *window;

    GtkWidget *vbox;
    GtkWidget *label_username;
    GtkWidget *mainbox;

    GtkWidget *left;
    GtkWidget *center;
    GtkWidget *rightbox;

    //center
    GtkWidget *imhtml;
    GtkWidget *imhtml_win;
    GtkWidget *center_hbox;
    GtkWidget *search_label;
    GtkWidget *search_entry;
    GtkWidget *search_button;
    
    //right
    RightStruct *showtable_struct;

    //left
    GtkWidget *left_scrolled;
    GtkTreeStore *treestore;
    GtkWidget *treeview;

    //conversation
    PidginConversation *gtkconv;

    //indicates end tag in send_iq_list 
    gboolean end_tag_set;

    /* number of retrieve conversations to show in imhtml */
    int number_of_convs_to_show;
    /* number of retrieve conversations saved in imhtml_list */
    int number_of_convs_saved;
    /* list of imhtml_messages to show */
    GList *imhtml_list;

    //xmlns for current server
    char *xmlns;

    //id for xmlns
    char *id;

    // retrieved collections
    GList *coll;

} WindowStruct;

typedef struct _NewCollection {
    gchar *date;
    gchar *start;
    gchar *with;
    gchar need_to_create_new;
} NewCollection;

typedef struct _ImhtmlText {
    gchar *date;
    gchar *text;
} ImhtmlText;

typedef struct {
    gchar *date;
    gchar *start;
    gchar *with;
    GList *to_retrieve;
} RetrieveCollection;

typedef struct {
    PidginConversation *gtkconv;
    gboolean included;
} Test_struct;

typedef struct {
    xmlnode *xml;
    char *id;
    gboolean match;
} Recipient_info;


extern PurplePlugin *xep136; 	/* plugin id pointer */
extern GList *list;		/* list of pointers to WindowStruct items */
extern char *xmlns_ejabberd;
extern char *xmlns_prosody;


extern void empty_collection(WindowStruct *curr);
extern void add_collection(WindowStruct *curr, gchar *start, gchar *with);
extern void history_button_clicked(GtkWidget *button, PidginConversation *gtkconv);

extern void free_imhtml_item(ImhtmlText *item);
extern int imhtml_compare_func(ImhtmlText *a, ImhtmlText *b);
extern void show_imhtml_conv(ImhtmlText *conv, WindowStruct *curr);
extern void imhtml_text_save_message(WindowStruct *curr, gchar *imhtml_message, gchar *secs, gchar *start);

extern gchar * increase_start_time(gchar *start);
extern gchar * get_friend_username(PidginConversation *gtkconv);
extern gchar * get_my_username(PidginConversation *gtkconv);
extern gchar * get_server_name(PidginConversation *gtkconv);
extern void find_recipient(WindowStruct *curr, Recipient_info *recipient);
extern int get_curr_year(void);

extern void retrieve_collection(WindowStruct *curr, gchar *date);
extern void explore_xml(WindowStruct *curr, xmlnode *xml);
extern void xmlnode_received(PurpleConnection *gc, xmlnode **packet, gpointer null);

extern void message_send(char *message, PidginConversation *gtkconv);
extern void send_iq_list(WindowStruct *curr, gchar *from, gchar *to);
extern void send_pref_info(WindowStruct *curr);
extern void status_clicked(GtkWidget *button, WindowStruct *curr);
extern void disable_clicked(GtkWidget *button, WindowStruct *curr);
extern void enable_clicked(GtkWidget *button, WindowStruct *curr);
extern void reset_clicked(GtkWidget *button, WindowStruct *curr);
extern void show_clicked(GtkWidget *button, WindowStruct *curr);
extern void send_disco_info(WindowStruct *curr);

#endif
