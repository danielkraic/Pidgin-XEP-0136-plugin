/*
 * xep136_gtk.c
 */

#include "xep136_plugin.h"

static void history_window_destroy(GtkWidget *window, WindowStruct *curr);
static void date_selected(GtkTreeSelection *sel, WindowStruct *curr);
static void search_clicked(GtkWidget *button, WindowStruct *curr);
static void create_right_table(WindowStruct *history_window);
static void create_left_list(WindowStruct *history_window);
static void history_window_create(WindowStruct *history_window);
static void history_window_open(PidginConversation *gtkconv);
static void history_window_exist_test(WindowStruct *curr, Test_struct *test);

/*------------------------------------------------------------
 * gtk related functions
 *------------------------------------------------------------*/

/* handle window "destroy" */
static void
history_window_destroy(GtkWidget *window, WindowStruct *curr)
{
    list = g_list_remove(list, curr);
    g_free(curr);
}

/* handle tree_store selection */
static void
date_selected(GtkTreeSelection *sel, WindowStruct *curr)
{
    GtkTreeIter iter;
    GtkTreeModel *model;
    gchar *date = NULL;

    /*
    if (curr->coll) {
	purple_debug_misc(PLUGIN_ID, "date_selected :: empty curr->coll\n");
	empty_curr_coll(curr);
    }
    */

    if (!gtk_tree_selection_get_selected(sel, &model, &iter)) {
	purple_debug_misc(PLUGIN_ID, "date_selected :: !gtk_tree_selection_get_selected\n");
	return;
    }

    gtk_imhtml_clear(GTK_IMHTML(curr->imhtml));
    
    gtk_tree_model_get(model, &iter, 0, &date, -1);

    if (!date) {
	purple_debug_misc(PLUGIN_ID, "date_selected :: !date\n");
	return;
    }

    retrieve_collection(curr, date);

    g_free(date);
}

/* handle imhtml searching */
static void
search_clicked(GtkWidget *button, WindowStruct *curr)
{
    gchar *search_text = NULL;

    search_text = (gchar *) gtk_entry_get_text(GTK_ENTRY(curr->search_entry));

    if ( (!search_text) || (strlen(search_text) == 0) ) {
	purple_debug_misc(PLUGIN_ID, "search_clicked :: !search_text\n");
	return;
    }

    gtk_imhtml_search_find(GTK_IMHTML(curr->imhtml), search_text);
}

/* create right box */
static void
create_right_table(WindowStruct *history_window)
{
    int i;
    int curr_year = 0;
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
    s->label_from = gtk_label_new("Date from:");
    s->label_to = gtk_label_new("Date to:");
    s->label_enable = gtk_label_new("Enable archiving:");
    s->label_disable = gtk_label_new("Disable archiving:");
    s->label_status = gtk_label_new("Show status:");
    s->label_preferences = gtk_label_new("\n\nXEP-0136 preferences:");

    //create combo boxes
    s->from_day = gtk_combo_box_new_text();
    s->from_month = gtk_combo_box_new_text();
    s->from_year = gtk_combo_box_new_text();

    s->to_day = gtk_combo_box_new_text();
    s->to_month = gtk_combo_box_new_text();
    s->to_year = gtk_combo_box_new_text();

    for (i = 1; i <= 31; i++) {
	text = g_strdup_printf("%d", i);
	gtk_combo_box_append_text(GTK_COMBO_BOX(s->from_day), (gchar *) text);
	gtk_combo_box_append_text(GTK_COMBO_BOX(s->to_day), (gchar *) text);
	g_free(text);
    }

    for (i = 1; i <= 12; i++) {
	text = g_strdup_printf("%d", i);
	gtk_combo_box_append_text(GTK_COMBO_BOX(s->from_month), (gchar *) text);
	gtk_combo_box_append_text(GTK_COMBO_BOX(s->to_month), (gchar *) text);
	g_free(text);
    }

    curr_year = get_curr_year();

    if (curr_year == 0) {
	curr_year = 2025;
	purple_debug_misc(PLUGIN_ID, "get_curr_year :: year = 2025\n");
    }

    for (i = 2004; i <= curr_year; i++) {
	text = g_strdup_printf("%d", i);
	gtk_combo_box_append_text(GTK_COMBO_BOX(s->from_year), (gchar *) text);
	gtk_combo_box_append_text(GTK_COMBO_BOX(s->to_year), (gchar *) text);
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

/* create left box */
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

    history_window->left_scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(history_window->left_scrolled),
	    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(history_window->left_scrolled), history_window->treeview);

    sel = gtk_tree_view_get_selection(GTK_TREE_VIEW (history_window->treeview));
    gtk_tree_selection_set_mode(sel, GTK_SELECTION_SINGLE);

    g_signal_connect (G_OBJECT (sel), "changed",
	    G_CALLBACK (date_selected), (gpointer) history_window);

    gtk_widget_set_size_request(GTK_WIDGET(history_window->left_scrolled), 165, -1);
}

/* create window */
static void
history_window_create(WindowStruct *history_window)
{
    gchar *username_text;

    history_window->window = pidgin_create_window("XEP-136 History", PIDGIN_HIG_BORDER, NULL, TRUE);
    gtk_window_set_default_size(GTK_WINDOW(history_window->window), 800, 350);

    g_signal_connect(G_OBJECT(history_window->window), "destroy", 
	    G_CALLBACK(history_window_destroy), (gpointer) history_window );

    //vbox
    history_window->vbox = gtk_vbox_new(FALSE, 3);

    //label
    username_text = g_strdup_printf("<span size='larger' weight='bold'>History for %s</span>", get_friend_username(history_window->gtkconv));
    history_window->label_username = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(history_window->label_username), username_text);
    gtk_misc_set_alignment(GTK_MISC(history_window->label_username), 0, 0);

    //left
    create_left_list(history_window);

    //center
    history_window->imhtml = gtk_imhtml_new(NULL, NULL);
    history_window->imhtml_win = gtk_scrolled_window_new(0, 0);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(history_window->imhtml_win), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    history_window->center_hbox = gtk_hbox_new(FALSE, 5);
    history_window->search_label = gtk_label_new("Search:");
    history_window->search_entry = gtk_entry_new();
    history_window->search_button = gtk_button_new_from_stock(GTK_STOCK_FIND);

    g_signal_connect(GTK_ENTRY(history_window->search_entry), "activate", 
	    G_CALLBACK(search_clicked), history_window);
    g_signal_connect(GTK_ENTRY(history_window->search_button), "clicked", 
	    G_CALLBACK(search_clicked), history_window);

    //right
    create_right_table(history_window);

    //boxing 
    history_window->mainbox = gtk_hbox_new(FALSE, 10);
    history_window->left = gtk_vbox_new(FALSE, 3);
    history_window->center = gtk_vbox_new(FALSE, 3);
    history_window->rightbox = gtk_vbox_new(FALSE, 3);

    //boxing left
    gtk_box_pack_start(GTK_BOX(history_window->left), history_window->left_scrolled, TRUE, TRUE, 0);

    //boxing center 
    gtk_box_pack_start(GTK_BOX(history_window->center_hbox), history_window->search_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(history_window->center_hbox), history_window->search_entry, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(history_window->center_hbox), history_window->search_button, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(history_window->imhtml_win), history_window->imhtml);
    gtk_box_pack_start(GTK_BOX(history_window->center), history_window->imhtml_win, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(history_window->center), history_window->center_hbox, FALSE, FALSE, 0);
    
    //boxing right
    gtk_box_pack_start(GTK_BOX(history_window->rightbox), (history_window->showtable_struct)->show_table, TRUE, FALSE, 0);

    //boxing main
    gtk_box_pack_start(GTK_BOX(history_window->mainbox), history_window->left, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(history_window->mainbox), history_window->center, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(history_window->mainbox), history_window->rightbox, FALSE, FALSE, 0);

    gtk_widget_set_sensitive(history_window->mainbox, FALSE);

    //boxing vbox
    gtk_box_pack_start(GTK_BOX(history_window->vbox), history_window->label_username, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(history_window->vbox), history_window->mainbox, TRUE, TRUE, 0);

    gtk_container_add(GTK_CONTAINER(history_window->window), history_window->vbox);
    gtk_widget_show_all(history_window->window);

    g_free(username_text);
}

/* init function before create window */
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
    /* set xmpp id */
    new->id = g_strdup_printf("xep136%x", g_random_int());

    history_window_create(new);

    /* add to list of windows */
    list = g_list_prepend(list, new);

    send_disco_info(new);
}

/* check if window already exist */
static void
history_window_exist_test(WindowStruct *curr, Test_struct *test)
{
    if (curr->gtkconv == test->gtkconv)
	test->included = TRUE;
}

/* handle history button */
void
history_button_clicked(GtkWidget *button, PidginConversation *gtkconv)
{
    Test_struct *test = g_malloc0(sizeof(Test_struct));

    test->gtkconv = gtkconv;
    test->included = FALSE;

    g_list_foreach(list, (GFunc) history_window_exist_test, (gpointer) test);

    if (test->included == TRUE) {
	g_free(test);
	//purple_debug_misc(PLUGIN_ID, "test included :: TRUE\n");
	return;
    }

    g_free(test);

    history_window_open(gtkconv);
}
