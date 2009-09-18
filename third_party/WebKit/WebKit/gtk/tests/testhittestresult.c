/*
 * Copyright (C) 2009 Igalia S.L.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2,1 of the License, or (at your option) any later version.
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
#include <glib/gstdio.h>
#include <webkit/webkit.h>

#if GTK_CHECK_VERSION(2, 14, 0)

typedef struct {
  char* data;
  guint flag;
} TestInfo;

static GMainLoop* loop;

typedef struct {
    WebKitWebView* webView;
    TestInfo* info;
} HitTestResultFixture;

TestInfo*
test_info_new(const char* data, guint flag)
{
    TestInfo* info;

    info = g_slice_new(TestInfo);
    info->data = g_strdup(data);
    info->flag = flag;

    return info;
}

void
test_info_destroy(TestInfo* info)
{
    g_free(info->data);
    g_slice_free(TestInfo, info);
}

static void hit_test_result_fixture_setup(HitTestResultFixture* fixture, gconstpointer data)
{
    fixture->webView = WEBKIT_WEB_VIEW(webkit_web_view_new());
    g_object_ref_sink(fixture->webView);
    loop = g_main_loop_new(NULL, TRUE);
    fixture->info = (TestInfo*)data;
}

static void hit_test_result_fixture_teardown(HitTestResultFixture* fixture, gconstpointer data)
{
    g_object_unref(fixture->webView);
    g_main_loop_unref(loop);
    test_info_destroy(fixture->info);
}

static void
load_status_cb(WebKitWebView* webView,
               GParamSpec* spec,
               gpointer data)
{
    WebKitLoadStatus status = webkit_web_view_get_load_status(webView);
    TestInfo* info = (TestInfo*)data;

    if (status == WEBKIT_LOAD_FINISHED) {
        WebKitHitTestResult* result;
        guint context;
        GdkEventButton event;
        event.x = GTK_WIDGET(webView)->allocation.width / 2;
        event.y = GTK_WIDGET(webView)->allocation.height / 2;

        result = webkit_web_view_get_hit_test_result(webView, &event);
        g_assert(result);
        g_object_get(result, "context", &context, NULL);
        g_assert(context & info->flag);
        g_object_unref(result);
        g_main_loop_quit(loop);
    }
}

static void
test_webkit_hit_test_result(HitTestResultFixture* fixture, gconstpointer data)
{
    TestInfo* info = (TestInfo*)data;
    GtkAllocation allocation = { 0, 0, 2, 2 };

    webkit_web_view_load_string(fixture->webView,
                                info->data,
                                "text/html",
                                "utf-8",
                                "file://");
    gtk_widget_size_allocate(GTK_WIDGET(fixture->webView), &allocation);
    g_signal_connect(fixture->webView, "notify::load-status", G_CALLBACK(load_status_cb), info);
    g_main_loop_run(loop);
}

int main(int argc, char** argv)
{
    g_thread_init(NULL);
    gtk_test_init(&argc, &argv, NULL);

    g_test_bug_base("https://bugs.webkit.org/");

    g_test_add("/webkit/hittestresult/document", HitTestResultFixture, 
               test_info_new("<html><body><h1>WebKitGTK+!</h1></body></html>",
                             WEBKIT_HIT_TEST_RESULT_CONTEXT_DOCUMENT),
               hit_test_result_fixture_setup, test_webkit_hit_test_result, hit_test_result_fixture_teardown);

/* We should really test this, but it's complicated to know where to
   ask for the coordinates in general without either DOM bindings or
   using JSC APIs */
#if 0
    g_test_add("/webkit/hittestresult/image", HitTestResultFixture,
               test_info_new("<html><body><img src='0xdeadbeef' width=50 height=50></img></body></html>",
                             WEBKIT_HIT_TEST_RESULT_CONTEXT_IMAGE),
               hit_test_result_fixture_setup, test_webkit_hit_test_result, hit_test_result_fixture_teardown);
    g_test_add("/webkit/hittestresult/editable", HitTestResultFixture,
               test_info_new("<html><body><input type='submit'></input>></body></html>",
                             WEBKIT_HIT_TEST_RESULT_CONTEXT_EDITABLE),
               hit_test_result_fixture_setup, test_webkit_hit_test_result, hit_test_result_fixture_teardown);
    g_test_add("/webkit/hittestresult/link", HitTestResultFixture,
               test_info_new("<html><body><a href='http://www.example.com'>HELLO WORLD</a></body></html>",
                             WEBKIT_HIT_TEST_RESULT_CONTEXT_LINK),
               hit_test_result_fixture_setup, test_webkit_hit_test_result, hit_test_result_fixture_teardown);
#endif
               
    return g_test_run ();
}

#else

int main(int argc, char** argv)
{
    g_critical("You will need at least GTK+ 2.14.0 to run the unit tests.");
    return 0;
}

#endif
