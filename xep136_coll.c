/*
 * xep136_coll.c
 */

#include "xep136_plugin.h"

static void add_collection_create_new(WindowStruct *curr, NewCollection *new);
static void add_collection_find(RetrieveCollection *curr_coll, NewCollection *new);

/*------------------------------------------------------------
 * collections related functions
 *------------------------------------------------------------*/

/* handle empty collection */
void
empty_collection(WindowStruct *curr)
{
    gtk_imhtml_clear(GTK_IMHTML(curr->imhtml));
    gtk_imhtml_append_text(GTK_IMHTML(curr->imhtml), "<b><font color='#cc0000'>Empty collection!</font></b><br>", 0);
}

/* create new collection to collections list */
static void
add_collection_create_new(WindowStruct *curr, NewCollection *new)
{
    RetrieveCollection *new_coll;
    GtkTreeIter iter;

    new_coll = (RetrieveCollection *) g_malloc0(sizeof(RetrieveCollection));
    if (!new_coll) {
	purple_debug_error(PLUGIN_ID, "ERROR: g_malloc0 new RetrieveCollection\n");
	return;
    }

    /* ceate new collection */
    new_coll->date = g_strdup(new->date);
    new_coll->start = g_strdup(new->start);
    new_coll->with = g_strdup(new->with);

    //purple_debug_misc(PLUGIN_ID, "add_collection_create_new :: %s :: %s :: %s\n", new_coll->date, new_coll->start, new_coll->with);

    new_coll->to_retrieve = g_list_prepend(new_coll->to_retrieve, new);

    /* add to the list of collections */
    curr->coll = g_list_prepend(curr->coll, new_coll);

    new->need_to_create_new = FALSE;

    /* append to tree_store with pretty date */
    gtk_tree_store_append(curr->treestore, &iter, NULL);
    gtk_tree_store_set(curr->treestore, &iter, 0, new_coll->date, -1);
}

/* find collection and add NewCollection to coll->to_retrieve */
static void
add_collection_find(RetrieveCollection *curr_coll, NewCollection *new)
{
    if (strncmp(curr_coll->date, new->date, 13) == 0) {

	//purple_debug_misc(PLUGIN_ID, "add_collection_find :: %s == %s\n", curr_coll->date, new->date);

	curr_coll->to_retrieve = g_list_prepend(curr_coll->to_retrieve, new);

	new->need_to_create_new = FALSE;
    }
}

/* add collection to collections list */
void
add_collection(WindowStruct *curr, gchar *start, gchar *with)
{
    NewCollection *new = NULL;

    new = (NewCollection *) g_malloc0(sizeof(NewCollection));
    if (!new) {
	purple_debug_error(PLUGIN_ID, "ERROR: g_malloc0 new NewCollection\n");
	return;
    }

    new->start = g_strdup(start);
    new->with = g_strdup(with);

    new->date = g_strndup(start, 16);
    new->date[10] = ' ';
    //new->date = g_strndup(start, 10);

    new->need_to_create_new = TRUE;

    if ( !(new->start) || !(new->with) || !(new->date) ) {
	purple_debug_misc(PLUGIN_ID, "add_collection :: !new\n");

	if (new->date)  g_free(new->date);
	if (new->start) g_free(new->start);
	if (new->with)  g_free(new->with);

	g_free(new);
	return;
    }

    purple_debug_misc(PLUGIN_ID, "#################### add_collection :: %s :: %s :: %s\n", new->date, new->start, new->with);

    if ( !(curr->coll) ) {
	//purple_debug_misc(PLUGIN_ID, "add_collection :: !(curr->coll)\n");
	add_collection_create_new(curr, new);
    } else {
	g_list_foreach(curr->coll, (GFunc) add_collection_find, (gpointer) new);

	if (new->need_to_create_new) {
	    //purple_debug_misc(PLUGIN_ID, "add_collection :: new->need_to_create_new\n");
	    add_collection_create_new(curr, new);
	}
    }
}
