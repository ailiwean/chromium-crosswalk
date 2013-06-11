# Copyright (c) 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'targets': [
    {
      'target_name': 'plugins',
      'type': '<(component)',
      'defines': [
        'WEBKIT_PLUGINS_IMPLEMENTATION',
      ],
      'include_dirs': [
        '<(INTERMEDIATE_DIR)',
        '<(SHARED_INTERMEDIATE_DIR)/webkit',
        '<(SHARED_INTERMEDIATE_DIR)/ui',
      ],
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/base/base.gyp:base_i18n',
        '<(DEPTH)/base/third_party/dynamic_annotations/dynamic_annotations.gyp:dynamic_annotations',
        '<(DEPTH)/gpu/gpu.gyp:command_buffer_common',
        '<(DEPTH)/gpu/gpu.gyp:gles2_c_lib',
        '<(DEPTH)/gpu/gpu.gyp:gles2_implementation',
        '<(DEPTH)/media/media.gyp:media',
        '<(DEPTH)/media/media.gyp:shared_memory_support',
        '<(DEPTH)/net/net.gyp:net',
        '<(DEPTH)/ppapi/ppapi.gyp:ppapi_c',
        '<(DEPTH)/ppapi/ppapi_internal.gyp:ppapi_shared',
        '<(DEPTH)/printing/printing.gyp:printing',
        '<(DEPTH)/skia/skia.gyp:skia',
        '<(DEPTH)/third_party/WebKit/public/blink.gyp:blink',
        '<(DEPTH)/ui/gl/gl.gyp:gl',
        '<(DEPTH)/ui/ui.gyp:ui',
        '<(DEPTH)/ui/ui.gyp:ui_resources',
        '<(DEPTH)/url/url.gyp:url_lib',
        '<(DEPTH)/v8/tools/gyp/v8.gyp:v8',
        '<(DEPTH)/webkit/base/webkit_base.gyp:webkit_base',
        '<(DEPTH)/webkit/common/user_agent/webkit_user_agent.gyp:user_agent',
        '<(DEPTH)/webkit/plugins/webkit_plugins.gyp:plugins_common',
        '<(DEPTH)/webkit/renderer/compositor_bindings/compositor_bindings.gyp:webkit_compositor_support',
        '<(DEPTH)/webkit/storage_common.gyp:webkit_storage_common',
        'glue_common',
        'webkit_common',
      ],
      'sources': [
        # This list contains all .h, .cc, and .mm files in glue except for
        # those in the test subdirectory and those with unittest in in their
        # names.
        '../plugins/npapi/gtk_plugin_container.cc',
        '../plugins/npapi/gtk_plugin_container.h',
        '../plugins/npapi/gtk_plugin_container_manager.cc',
        '../plugins/npapi/gtk_plugin_container_manager.h',
        '../plugins/npapi/plugin_host.cc',
        '../plugins/npapi/plugin_host.h',
        '../plugins/npapi/plugin_instance.cc',
        '../plugins/npapi/plugin_instance.h',
        '../plugins/npapi/plugin_instance_mac.mm',
        '../plugins/npapi/plugin_lib.cc',
        '../plugins/npapi/plugin_lib.h',
        '../plugins/npapi/plugin_stream.cc',
        '../plugins/npapi/plugin_stream.h',
        '../plugins/npapi/plugin_stream_posix.cc',
        '../plugins/npapi/plugin_stream_url.cc',
        '../plugins/npapi/plugin_stream_url.h',
        '../plugins/npapi/plugin_stream_win.cc',
        '../plugins/npapi/plugin_string_stream.cc',
        '../plugins/npapi/plugin_string_stream.h',
        '../plugins/npapi/plugin_web_event_converter_mac.h',
        '../plugins/npapi/plugin_web_event_converter_mac.mm',
        '../plugins/npapi/webplugin.cc',
        '../plugins/npapi/webplugin.h',
        '../plugins/npapi/webplugin_accelerated_surface_mac.h',
        '../plugins/npapi/webplugin_delegate.h',
        '../plugins/npapi/webplugin_delegate_impl.cc',
        '../plugins/npapi/webplugin_delegate_impl.h',
        '../plugins/npapi/webplugin_delegate_impl_android.cc',
        '../plugins/npapi/webplugin_delegate_impl_aura.cc',
        '../plugins/npapi/webplugin_delegate_impl_gtk.cc',
        '../plugins/npapi/webplugin_delegate_impl_mac.mm',
        '../plugins/npapi/webplugin_delegate_impl_win.cc',
        '../plugins/npapi/webplugin_ime_win.cc',
        '../plugins/npapi/webplugin_ime_win.h',
        '../plugins/npapi/webplugin_impl.cc',
        '../plugins/npapi/webplugin_impl.h',
        '../plugins/ppapi/audio_helper.cc',
        '../plugins/ppapi/audio_helper.h',
        '../plugins/ppapi/common.h',
        '../plugins/ppapi/content_decryptor_delegate.cc',
        '../plugins/ppapi/content_decryptor_delegate.h',
        '../plugins/ppapi/event_conversion.cc',
        '../plugins/ppapi/event_conversion.h',
        '../plugins/ppapi/fullscreen_container.h',
        '../plugins/ppapi/gfx_conversion.h',
        '../plugins/ppapi/host_array_buffer_var.cc',
        '../plugins/ppapi/host_array_buffer_var.h',
        '../plugins/ppapi/host_globals.cc',
        '../plugins/ppapi/host_globals.h',
        '../plugins/ppapi/host_var_tracker.cc',
        '../plugins/ppapi/host_var_tracker.h',
        '../plugins/ppapi/message_channel.cc',
        '../plugins/ppapi/message_channel.h',
        '../plugins/ppapi/npapi_glue.cc',
        '../plugins/ppapi/npapi_glue.h',
        '../plugins/ppapi/npobject_var.cc',
        '../plugins/ppapi/npobject_var.h',
        '../plugins/ppapi/plugin_delegate.h',
        '../plugins/ppapi/plugin_module.cc',
        '../plugins/ppapi/plugin_module.h',
        '../plugins/ppapi/plugin_object.cc',
        '../plugins/ppapi/plugin_object.h',
        '../plugins/ppapi/ppapi_interface_factory.cc',
        '../plugins/ppapi/ppapi_interface_factory.h',
        '../plugins/ppapi/ppapi_plugin_instance.cc',
        '../plugins/ppapi/ppapi_plugin_instance.h',
        '../plugins/ppapi/ppapi_webplugin_impl.cc',
        '../plugins/ppapi/ppapi_webplugin_impl.h',
        '../plugins/ppapi/ppb_audio_impl.cc',
        '../plugins/ppapi/ppb_audio_impl.h',
        '../plugins/ppapi/ppb_broker_impl.cc',
        '../plugins/ppapi/ppb_broker_impl.h',
        '../plugins/ppapi/ppb_buffer_impl.cc',
        '../plugins/ppapi/ppb_buffer_impl.h',
        '../plugins/ppapi/ppb_file_ref_impl.cc',
        '../plugins/ppapi/ppb_file_ref_impl.h',
        '../plugins/ppapi/ppb_flash_message_loop_impl.cc',
        '../plugins/ppapi/ppb_flash_message_loop_impl.h',
        '../plugins/ppapi/ppb_gpu_blacklist_private_impl.cc',
        '../plugins/ppapi/ppb_gpu_blacklist_private_impl.h',
        '../plugins/ppapi/ppb_graphics_3d_impl.cc',
        '../plugins/ppapi/ppb_graphics_3d_impl.h',
        '../plugins/ppapi/ppb_image_data_impl.cc',
        '../plugins/ppapi/ppb_image_data_impl.h',
        '../plugins/ppapi/ppb_network_monitor_private_impl.cc',
        '../plugins/ppapi/ppb_network_monitor_private_impl.h',
        '../plugins/ppapi/ppb_proxy_impl.cc',
        '../plugins/ppapi/ppb_proxy_impl.h',
        '../plugins/ppapi/ppb_scrollbar_impl.cc',
        '../plugins/ppapi/ppb_scrollbar_impl.h',
        '../plugins/ppapi/ppb_tcp_server_socket_private_impl.cc',
        '../plugins/ppapi/ppb_tcp_server_socket_private_impl.h',
        '../plugins/ppapi/ppb_tcp_socket_private_impl.cc',
        '../plugins/ppapi/ppb_tcp_socket_private_impl.h',
        '../plugins/ppapi/ppb_uma_private_impl.cc',
        '../plugins/ppapi/ppb_uma_private_impl.h',
        '../plugins/ppapi/ppb_var_deprecated_impl.cc',
        '../plugins/ppapi/ppb_var_deprecated_impl.h',
        '../plugins/ppapi/ppb_video_decoder_impl.cc',
        '../plugins/ppapi/ppb_video_decoder_impl.h',
        '../plugins/ppapi/ppb_widget_impl.cc',
        '../plugins/ppapi/ppb_widget_impl.h',
        '../plugins/ppapi/ppb_x509_certificate_private_impl.cc',
        '../plugins/ppapi/ppb_x509_certificate_private_impl.h',
        '../plugins/ppapi/quota_file_io.cc',
        '../plugins/ppapi/quota_file_io.h',
        '../plugins/ppapi/resource_creation_impl.cc',
        '../plugins/ppapi/resource_creation_impl.h',
        '../plugins/ppapi/resource_helper.cc',
        '../plugins/ppapi/resource_helper.h',
        '../plugins/ppapi/string.cc',
        '../plugins/ppapi/string.h',
        '../plugins/ppapi/url_response_info_util.cc',
        '../plugins/ppapi/url_response_info_util.h',
        '../plugins/ppapi/url_request_info_util.cc',
        '../plugins/ppapi/url_request_info_util.h',
        '../plugins/ppapi/usb_key_code_conversion.h',
        '../plugins/ppapi/usb_key_code_conversion.cc',
        '../plugins/ppapi/usb_key_code_conversion_linux.cc',
        '../plugins/ppapi/usb_key_code_conversion_mac.cc',
        '../plugins/ppapi/usb_key_code_conversion_win.cc',
        '../plugins/ppapi/v8_var_converter.cc',
        '../plugins/ppapi/v8_var_converter.h',
        '../plugins/sad_plugin.cc',
        '../plugins/sad_plugin.h',
        '../plugins/webkit_plugins_export.h',
      ],
      'conditions': [
        ['toolkit_uses_gtk == 1', {
          'dependencies': [
            '<(DEPTH)/build/linux/system.gyp:gtk',
          ],
          'sources/': [['exclude', '_x11\\.cc$']],
        }],
        ['use_aura==1', {
          'sources/': [
            ['exclude', '^\\.\\./plugins/npapi/webplugin_delegate_impl_mac.mm'],
          ],
        }],
        ['use_aura==1 and OS=="win"', {
          'sources/': [
            ['exclude', '^\\.\\./plugins/npapi/webplugin_delegate_impl_aura'],
          ],
        }],
        ['OS!="mac"', {
          'sources/': [['exclude', '_mac\\.(cc|mm)$']],
        }, {  # else: OS=="mac"
          'link_settings': {
            'libraries': [
              '$(SDKROOT)/System/Library/Frameworks/QuartzCore.framework',
            ],
          },
        }],
        ['enable_gpu!=1', {
          'sources!': [
            '../plugins/ppapi/ppb_graphics_3d_impl.cc',
            '../plugins/ppapi/ppb_graphics_3d_impl.h',
            '../plugins/ppapi/ppb_open_gl_es_impl.cc',
          ],
        }],
        ['OS!="win"', {
          'sources/': [['exclude', '_win\\.cc$']],
        }, {  # else: OS=="win"
          # TODO(jschuh): crbug.com/167187 fix size_t to int truncations.
          'msvs_disabled_warnings': [ 4800, 4267 ],
          'sources/': [['exclude', '_posix\\.cc$']],
          'include_dirs': [
            '<(DEPTH)/third_party/wtl/include',
          ],
        }],
      ],
    },
  ]
}
