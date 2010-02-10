/*
 * Copyright (C) 2008 Holger Hans Peter Freyther
 * Copyright (C) 2009 Collabora Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <errno.h>
#include <unistd.h>
#include <string.h>

#include <glib.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>
#include <webkit/webkit.h>

#if GLIB_CHECK_VERSION(2, 16, 0) && GTK_CHECK_VERSION(2, 14, 0)

GMainLoop* loop;
SoupSession *session;
char* base_uri;

/* For real request testing */
static void
server_callback(SoupServer* server, SoupMessage* msg,
                 const char* path, GHashTable* query,
                 SoupClientContext* context, gpointer data)
{
    if (msg->method != SOUP_METHOD_GET) {
        soup_message_set_status(msg, SOUP_STATUS_NOT_IMPLEMENTED);
        return;
    }

    soup_message_set_status(msg, SOUP_STATUS_OK);

    if (g_str_equal(path, "/favicon.ico")) {
        char* contents;
        gsize length;
        GError* error = NULL;

        g_file_get_contents("blank.ico", &contents, &length, &error);
        g_assert(!error);

        soup_message_body_append(msg->response_body, SOUP_MEMORY_TAKE, contents, length);
    } else if (g_str_equal(path, "/bigdiv.html")) {
        char* contents = g_strdup("<html><body><div style=\"background-color: green; height: 1200px;\"></div></body></html>");
        soup_message_body_append(msg->response_body, SOUP_MEMORY_TAKE, contents, strlen(contents));
    } else if (g_str_equal(path, "/iframe.html")) {
        char* contents = g_strdup("<html><body><div style=\"background-color: green; height: 50px;\"></div><iframe src=\"bigdiv.html\"></iframe></body></html>");
        soup_message_body_append(msg->response_body, SOUP_MEMORY_TAKE, contents, strlen(contents));
    } else {
        char* contents = g_strdup("<html><body>test</body></html>");
        soup_message_body_append(msg->response_body, SOUP_MEMORY_TAKE, contents, strlen(contents));
    }

    soup_message_body_complete(msg->response_body);
}

static void idle_quit_loop_cb(WebKitWebView* web_view, GParamSpec* pspec, gpointer data)
{
    if (webkit_web_view_get_load_status(web_view) == WEBKIT_LOAD_FINISHED ||
        webkit_web_view_get_load_status(web_view) == WEBKIT_LOAD_FAILED)
        g_main_loop_quit(loop);
}

static void icon_uri_changed_cb(WebKitWebView* web_view, GParamSpec* pspec, gpointer data)
{
    gboolean* been_here = (gboolean*)data;
    char* expected_uri;

    g_assert_cmpstr(g_param_spec_get_name(pspec), ==, "icon-uri");

    expected_uri = g_strdup_printf("%sfavicon.ico", base_uri);
    g_assert_cmpstr(webkit_web_view_get_icon_uri(web_view), ==, expected_uri);
    g_free(expected_uri);

    *been_here = TRUE;
}

static void icon_loaded_cb(WebKitWebView* web_view, char* icon_uri, gpointer data)
{
    gboolean* been_here = (gboolean*)data;
    char* expected_uri = g_strdup_printf("%sfavicon.ico", base_uri);
    g_assert_cmpstr(icon_uri, ==, expected_uri);
    g_free(expected_uri);

    g_assert_cmpstr(icon_uri, ==, webkit_web_view_get_icon_uri(web_view));

    *been_here = TRUE;
}

static void test_webkit_web_view_icon_uri()
{
    gboolean been_to_uri_changed = FALSE;
    gboolean been_to_icon_loaded = FALSE;
    WebKitWebView* view = WEBKIT_WEB_VIEW(webkit_web_view_new());
    g_object_ref_sink(G_OBJECT(view));

    loop = g_main_loop_new(NULL, TRUE);

    g_object_connect(G_OBJECT(view),
                     "signal::notify::progress", idle_quit_loop_cb, NULL,
                     "signal::notify::icon-uri", icon_uri_changed_cb, &been_to_uri_changed,
                     "signal::icon-loaded", icon_loaded_cb, &been_to_icon_loaded,
                     NULL);

    webkit_web_view_load_uri(view, base_uri);

    g_main_loop_run(loop);

    g_assert(been_to_uri_changed);
    g_assert(been_to_icon_loaded);

    g_object_unref(view);
}

static gboolean map_event_cb(GtkWidget *widget, GdkEvent* event, gpointer data)
{
    GMainLoop* loop = (GMainLoop*)data;
    g_main_loop_quit(loop);

    return FALSE;
}

static void do_test_webkit_web_view_adjustments(gboolean with_page_cache)
{
    char* effective_uri = g_strconcat(base_uri, "bigdiv.html", NULL);
    char* second_uri = g_strconcat(base_uri, "iframe.html", NULL);
    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    GtkWidget* scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    WebKitWebView* view = WEBKIT_WEB_VIEW(webkit_web_view_new());
    GtkAdjustment* adjustment;
    double lower;
    double upper;

    if (with_page_cache) {
        WebKitWebSettings* settings = webkit_web_view_get_settings(view);
        g_object_set(settings, "enable-page-cache", TRUE, NULL);
    }

    gtk_window_set_default_size(GTK_WINDOW(window), 400, 200);

    gtk_container_add(GTK_CONTAINER(window), scrolled_window);
    gtk_container_add(GTK_CONTAINER(scrolled_window), GTK_WIDGET(view));

    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    loop = g_main_loop_new(NULL, TRUE);

    g_object_connect(G_OBJECT(view),
                     "signal::notify::progress", idle_quit_loop_cb, NULL,
                     NULL);

    /* Wait for window to show up */
    gtk_widget_show_all(window);
    g_signal_connect(window, "map-event",
                     G_CALLBACK(map_event_cb), loop);
    g_main_loop_run(loop);

    /* Load a page with a big div that will cause scrollbars to appear */
    webkit_web_view_load_uri(view, effective_uri);
    g_main_loop_run(loop);

    adjustment = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(scrolled_window));
    g_assert_cmpfloat(gtk_adjustment_get_value(adjustment), ==, 0.0);

    lower = gtk_adjustment_get_lower(adjustment);
    upper = gtk_adjustment_get_upper(adjustment);

    /* Scroll the view using JavaScript */
    webkit_web_view_execute_script(view, "window.scrollBy(0, 100)");

    /* Make sure the ScrolledWindow noticed the scroll */
    g_assert_cmpfloat(gtk_adjustment_get_value(adjustment), ==, 100.0);

    /* Load a second URI */
    webkit_web_view_load_uri(view, second_uri);
    g_main_loop_run(loop);

    /* Make sure the scrollbar has been reset */
    g_assert_cmpfloat(gtk_adjustment_get_value(adjustment), ==, 0.0);

    /* Go back */
    webkit_web_view_go_back(view);

    /* When using page cache, go_back will return syncronously */
    if (!with_page_cache)
        g_main_loop_run(loop);

    /* Make sure GTK+ has time to process the changes in size, for the adjusments */
    while (gtk_events_pending())
        gtk_main_iteration();

    /* Make sure upper and lower bounds have been restored correctly */
    g_assert_cmpfloat(lower, ==, gtk_adjustment_get_lower(adjustment));
    g_assert_cmpfloat(upper, ==, gtk_adjustment_get_upper(adjustment));

    g_assert_cmpfloat(gtk_adjustment_get_value(adjustment), ==, 100.0);

    g_free(effective_uri);
    g_free(second_uri);

    gtk_widget_destroy(window);
}

static void test_webkit_web_view_adjustments()
{
    /* Test this with page cache disabled, and enabled. */
    do_test_webkit_web_view_adjustments(FALSE);
    do_test_webkit_web_view_adjustments(TRUE);
}

int main(int argc, char** argv)
{
    SoupServer* server;
    SoupURI* soup_uri;

    g_thread_init(NULL);
    gtk_test_init(&argc, &argv, NULL);

    /* Hopefully make test independent of the path it's called from. */
    while (!g_file_test ("WebKit/gtk/tests/resources/test.html", G_FILE_TEST_EXISTS)) {
        char path_name[PATH_MAX];

        g_chdir("..");

        g_assert(!g_str_equal(getcwd(path_name, PATH_MAX), "/"));
    }

    g_chdir("WebKit/gtk/tests/resources/");

    server = soup_server_new(SOUP_SERVER_PORT, 0, NULL);
    soup_server_run_async(server);

    soup_server_add_handler(server, NULL, server_callback, NULL, NULL);

    soup_uri = soup_uri_new("http://127.0.0.1/");
    soup_uri_set_port(soup_uri, soup_server_get_port(server));

    base_uri = soup_uri_to_string(soup_uri, FALSE);
    soup_uri_free(soup_uri);

    g_test_bug_base("https://bugs.webkit.org/");
    g_test_add_func("/webkit/webview/icon-uri", test_webkit_web_view_icon_uri);
    g_test_add_func("/webkit/webview/adjustments", test_webkit_web_view_adjustments);

    return g_test_run ();
}

#else
int main(int argc, char** argv)
{
    g_critical("You will need at least glib-2.16.0 and gtk-2.14.0 to run the unit tests. Doing nothing now.");
    return 0;
}

#endif
