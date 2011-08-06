#define PURPLE_PLUGINS

#include <glib.h>

#include "notify.h"
#include "plugin.h"
#include "version.h"

/* we're adding this here and assigning it in plugin_load because we need
 * a valid plugin handle for our call to purple_notify_message() in the
 * plugin_action_test_cb() callback function */
static PurplePlugin *xep136_plugin_pointer = NULL;

static void
notify_format_cb(PurplePluginAction *action)
{
    purple_notify_formatted(xep136_plugin_pointer, "Test notifikacie", "Test pre xep136", 
	    "Test xep136",
	    "<I>Test i formated bla bla.</I>", NULL, NULL);
}

/* we tell libpurple in the PurplePluginInfo struct to call this function to
 * get a list of plugin actions to use for the plugin.  This function gives
 * libpurple that list of actions. */
static GList *
plugin_actions(PurplePlugin *plugin, gpointer context)
{
    GList *actions = NULL;

    actions = g_list_prepend(actions, 
	    purple_plugin_action_new("Moj plugin cez Formated notification", notify_format_cb));

    /* Once the list is complete, we send it to libpurple. */
    return actions;
}

static gboolean
plugin_load(PurplePlugin *plugin)
{
    purple_notify_message(plugin, PURPLE_NOTIFY_MSG_INFO, "Hello World!",
            "This is XEP-136 plugin :)", NULL, NULL, NULL);

    xep136_plugin_pointer = plugin; /* assign this here so we have a valid handle later */

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
    "XEP-136 plugin!",
    "1.1",

    "XEP-136 plugin",
    "XEP-136 plugin",
    "Daniel Kraic <danielkraic@gmail.com>",
    "http://www.denko.yw.sk",


    plugin_load,
    NULL,
    NULL,

    NULL,
    NULL,
    NULL,
    plugin_actions, /* this tells libpurple the address of the function to call
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
