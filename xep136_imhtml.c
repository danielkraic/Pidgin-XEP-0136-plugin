/*
 * xep136_imhtml.c
 */

#include "xep136_plugin.h"

static gchar * imhtml_text_make_date(gchar *secs, gchar *start);

/*------------------------------------------------------------
 * imhtml related functions
 *------------------------------------------------------------*/

/* free ImhtmlText item from imhtml_list */
void
free_imhtml_item(ImhtmlText *item)
{
    g_free(item->date);
    g_free(item->text);
}

/* compare dates of ImhtmlText items */
int 
imhtml_compare_func(ImhtmlText *a, ImhtmlText *b)
{
    return strcmp(a->date, b->date);
}

/* show ImhtmlText item */
void
show_imhtml_conv(ImhtmlText *conv, WindowStruct *curr)
{
    gtk_imhtml_append_text(GTK_IMHTML(curr->imhtml), conv->date, 0);
    gtk_imhtml_append_text(GTK_IMHTML(curr->imhtml), " :: ", 0);
    gtk_imhtml_append_text(GTK_IMHTML(curr->imhtml), conv->text, 0);
}

/* create string date+time for imhtml item */
static gchar *
imhtml_text_make_date(gchar *secs, gchar *start)
{
    char date[11];
    char hour[3];
    char minute[3];
    char second[3];
    gchar *result = NULL;
    int hou;
    int min;
    int sec;
    int secs_int = atoi(secs);

    strncpy(date, start, 10);
    date[10] = '\0';

    hour[0] = start[11];
    hour[1] = start[12];
    hour[2] = '\0';
    hou = atoi(hour);

    minute[0] = start[14];
    minute[1] = start[15];
    minute[2] = '\0';
    min = atoi(minute);

    second[0] = start[17];
    second[1] = start[18];
    second[2] = '\0';
    sec = atoi(second);

    /* add secs to date */
    min += (int) ( (secs_int + sec) / 60 );
    sec = (int) ( (secs_int + sec) % 60 );

    hou += (int) (min / 60);
    min = (int) (min % 60);
    
    //result = g_strdup_printf("%s %c%c:%c%c:%c%c", date, hour[0], hour[1], minute[0], minute[1], second[0], second[1]);
    result = g_strdup_printf("%s %02d:%02d:%02d", date, hou, min, sec);

    return result;
}

/* save item to ImhtmlText list */
void
imhtml_text_save_message(WindowStruct *curr, gchar *imhtml_message, gchar *secs, gchar *start)
{
    ImhtmlText *new = NULL;

    if (!start) {
	purple_debug_error(PLUGIN_ID, "ERROR: !start imhtml_text_save_message\n");
	return;
    }

    if (!secs) {
	purple_debug_error(PLUGIN_ID, "ERROR: !secs imhtml_text_save_message\n");
	return;
    }
    if (!imhtml_message) {
	purple_debug_error(PLUGIN_ID, "ERROR: !imhtml_message imhtml_text_save_message\n");
	return;
    }

    new  = (ImhtmlText *) g_malloc0(sizeof(ImhtmlText));
    if (!new) {
	purple_debug_error(PLUGIN_ID, "ERROR: g_malloc0 !new ImhtmlText imhtml_text_save_message\n");
	return;
    }

    new->date = imhtml_text_make_date(secs, start);
    new->text = g_strdup(imhtml_message);

    if (!new->date) {
	purple_debug_error(PLUGIN_ID, "ERROR: !new->date imhtml_text_save_message\n");
	return;
    }

    if (!new->text) {
	purple_debug_error(PLUGIN_ID, "ERROR: !new->text imhtml_text_save_message\n");
	return;
    }

    curr->imhtml_list = g_list_prepend(curr->imhtml_list, new);
}
