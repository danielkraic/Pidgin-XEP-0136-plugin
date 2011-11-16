/* xep136_plugin.h */

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
//    PurpleConnection *gc;
    xmlnode *xml;
    char *id;
    gboolean match;
} Recipient_info;

/* misc functions, increase_start_time, get_server_name, find_recipient */
static gchar * make_pretty_date(gchar *raw);
static gchar * make_raw_date(gchar *pretty);
static gchar * increase_start_time(gchar *start);
static gchar * get_my_username(PidginConversation *gtkconv);
static gchar * get_server_name(PidginConversation *gtkconv);
static void find_recipient(WindowStruct *curr, Recipient_info *recipient);
static int get_curr_year(void);
static void free_imhtml_item(ImhtmlText *item);
static int imhtml_compare_func(ImhtmlText *a, ImhtmlText *b);
static void show_imhtml_conv(ImhtmlText *conv, WindowStruct *curr);

/* explore received xmlnode, manage collections */
static void send_propher_name(RetrieveCollection *coll, RetrieveCollection *new);
static void retrieve_collection_send_message(NewCollection *new, WindowStruct *curr);
static void retrieve_collection_find(RetrieveCollection *curr, RetrieveCollection *new);
static void retrieve_collection(WindowStruct *curr, gchar *date);
static gchar * imhtml_text_make_date(gchar *secs, gchar *start);
static void imhtml_text_save_message(WindowStruct *curr, gchar *imhtml_message, gchar *secs, gchar *start);
static void iq_retrieve_body(WindowStruct *curr, xmlnode *c, xmlnode *d, gchar *secs, gchar *start);
static void iq_retrieve(WindowStruct *curr, xmlnode *xml);
static void empty_collection(WindowStruct *curr);
static void add_collection_create_new(WindowStruct *curr, NewCollection *new);
static void add_collection_find(RetrieveCollection *curr_coll, NewCollection *new);
static void add_collection(WindowStruct *curr, gchar *start, gchar *with);
static void iq_list(WindowStruct *curr, xmlnode *xml);
static void iq_pref(WindowStruct *curr, xmlnode *xml);
static void iq_query_supported(WindowStruct *curr);
static void iq_query(WindowStruct *curr, xmlnode *xml);
static void explore_xml(WindowStruct *curr, xmlnode *xml);
static void xmlnode_received(PurpleConnection *gc, xmlnode **packet, gpointer null);

/* send message, service discovery, show, enable, disable, status */
static void message_send(char *message, PidginConversation *gtkconv);
static void send_iq_list(WindowStruct *curr, gchar *from, gchar *to);
static void send_pref_info(WindowStruct *curr);
static void status_clicked(GtkWidget *button, WindowStruct *curr);
static void disable_clicked(GtkWidget *button, WindowStruct *curr);
static void enable_clicked(GtkWidget *button, WindowStruct *curr);
static void reset_clicked(GtkWidget *button, WindowStruct *curr);
static gchar *show_clicked_make_to(RightStruct *s);
static gchar *show_clicked_make_from(RightStruct *s);
static void show_clicked(GtkWidget *button, WindowStruct *curr);
static void send_disco_info(WindowStruct *curr);

/* GTK create, destroy, history window */
static void history_window_destroy(GtkWidget *window, WindowStruct *curr);
static void date_selected(GtkTreeSelection *sel, WindowStruct *curr);
static void search_clicked(GtkWidget *button, WindowStruct *curr);
static void create_right_table(WindowStruct *history_window);
static void create_left_list(WindowStruct *history_window);
static void history_window_create(WindowStruct *history_window);
static void history_window_open(PidginConversation *gtkconv);
static void history_window_exist_test(WindowStruct *curr, Test_struct *test);
static void history_button_clicked(GtkWidget *button, PidginConversation *gtkconv);

/* attach, detach, history_button */
static gboolean if_jabber(PidginConversation *gtkconv);
static void destroy_windows(WindowStruct *curr);
static void destroy_history_window(WindowStruct *curr, PidginConversation *gtkconv);
static void conv_deleted(PurpleConversation *conv, gpointer null);
static void detach_from_gtkconv(PidginConversation *gtkconv, gpointer null);
static void attach_to_gtkconv(PidginConversation *gtkconv, gpointer null);
static void detach_from_pidgin_window(PidginWindow *win, gpointer null);
static void attach_to_pidgin_window(PidginWindow *win, gpointer null);
static void detach_from_all_windows();
static void attach_to_all_windows();
static void conv_created(PurpleConversation *conv, gpointer null);
