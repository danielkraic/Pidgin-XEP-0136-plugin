/* 
 * xep136_misc.c
 */

#include "xep136_plugin.h"

/*--------------------------------------------------------------------------------
 * misc functions: increase_start_time, get_server_name, find_recipient, ...
 *--------------------------------------------------------------------------------*/

/* increase start time by one second */
gchar *
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

/* return friend's username (jid) from conversation */
gchar *
get_friend_username(PidginConversation *gtkconv)
{
    PurpleConversation *conv = gtkconv->active_conv;

    if (!conv)
	return NULL;

    return conv->name;
}

/* return user's username (jid) from PurpleAccount */
gchar *
get_my_username(PidginConversation *gtkconv)
{
    PurpleConversation *purple_conv = gtkconv->active_conv;
    PurpleAccount *acc = purple_conv->account;
    char *username= acc->username;

    gchar *my_username = NULL;;
    gchar *lom = NULL; //pointer to '/'
    glong dlzka = 0;

    if (!acc)
	return NULL;

    dlzka = g_utf8_strlen(username, -1);

    lom = g_strstr_len(username, dlzka, "/");

    if (lom != NULL) {
	dlzka = dlzka - g_utf8_strlen(lom, -1);
    }

    my_username = (gchar *) g_malloc0( (dlzka + 1) * sizeof(gchar) );

    g_strlcat (my_username, username, dlzka + 1);

    return my_username;
}

/* return server name from users's jid from PurpleAccount */
gchar * 
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

/* find users match from xmpp message id */
void
find_recipient(WindowStruct *curr, Recipient_info *recipient)
{
    /* end if already matched */
    if (recipient->match == TRUE)
	return;

    //purple_debug_misc(PLUGIN_ID, "find_recipient :: curr->id=%s recipient->id=%s\n", curr->id, recipient->id);

    if (strcmp(curr->id, recipient->id) == 0) {
	recipient->match = TRUE;
    	explore_xml(curr, recipient->xml);
    }
}

/* return current year as int */
int
get_curr_year(void)
{
    struct tm curr_time;
    time_t raw;
    int year;

    time(&raw);

    localtime_r(&raw, &curr_time);

    year = (int) (1900 + (int) curr_time.tm_year);

    //purple_debug_misc(PLUGIN_ID, "get_curr_year :: %d\n", year);

    return year;
}


