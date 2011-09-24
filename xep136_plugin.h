/* xep136_plugin.h */

typedef struct _WindowStruct {
    GtkWidget *window;
    GtkWidget *mainbox;
    GtkWidget *rightbox;
    GtkWidget *hbox;
    GtkWidget *vbox;
    GtkWidget *label_username;
    GtkWidget *imhtml;
    GtkWidget *label;
    GtkWidget *entry;
    GtkWidget *button;
    GtkWidget *show;
    GtkWidget *next;
    GtkWidget *enable;
    GtkWidget *disable;
    PidginConversation *gtkconv;
    char *xmlns;
} WindowStruct;

typedef struct {
    PidginConversation *gtkconv;
    gboolean included;
} Test_struct;

typedef struct _Recipient_info {
    PurpleConnection *gc;
    xmlnode *xml;
    gboolean match;
} Recipient_info;

 
/* misc functions, get_my_username, get_server_name, find_recipient */
//static char * get_my_username(WindowStruct *curr);
static gchar * get_server_name(PidginConversation *gtkconv);
static void find_recipient(WindowStruct *curr, Recipient_info *recipient);

/* receive and explore received xmlnode, iq_list, iq_query */
//static void retrieve_collection(WindowStruct *curr, char *start);
static void iq_list(WindowStruct *curr, xmlnode *xml);
static void iq_query(WindowStruct *curr, xmlnode *xml);
static void explore_xml(WindowStruct *curr, xmlnode *xml);
static void xmlnode_received(PurpleConnection *gc, xmlnode **packet, gpointer null);

/* send message, service discovery, list collections */
static void message_send(char *message, PidginConversation *gtkconv);
static void show_clicked(GtkWidget *button, WindowStruct *curr);
static void send_disco_info(PidginConversation *gtkconv);

/* GTK create, destroy, history window */
static void history_window_destroy(GtkWidget *window, WindowStruct *curr);
static void history_window_create(WindowStruct *history_window);
static void history_window_open(PidginConversation *gtkconv);
static void history_window_exist_test(WindowStruct *curr, Test_struct *test);
static void history_button_clicked(GtkWidget *button, PidginConversation *gtkconv);

/* attach, detach, history_button */
static gboolean if_jabber(PidginConversation *gtkconv);
static void detach_from_gtkconv(PidginConversation *gtkconv, gpointer null);
static void attach_to_gtkconv(PidginConversation *gtkconv, gpointer null);
static void detach_from_pidgin_window(PidginWindow *win, gpointer null);
static void attach_to_pidgin_window(PidginWindow *win, gpointer null);
static void detach_from_all_windows();
static void attach_to_all_windows();
static void conv_created(PurpleConversation *conv, gpointer null);
static void destroy_windows(WindowStruct *curr);
