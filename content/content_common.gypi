# Copyright (c) 2011 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'dependencies': [
    '../base/base.gyp:base',
    '../build/temp_gyp/googleurl.gyp:googleurl',
    '../gpu/gpu.gyp:gpu_ipc',
    '../ipc/ipc.gyp:ipc',
    '../media/media.gyp:media',
    '../net/net.gyp:net',
    '../skia/skia.gyp:skia',
    '../third_party/icu/icu.gyp:icuuc',
    '../third_party/npapi/npapi.gyp:npapi',
    '../third_party/WebKit/Source/WebKit/chromium/WebKit.gyp:webkit',
    '../ui/gfx/gl/gl.gyp:gl',
    '../ui/ui.gyp:ui',
    '../webkit/support/webkit_support.gyp:appcache',
    '../webkit/support/webkit_support.gyp:blob',
    '../webkit/support/webkit_support.gyp:database',
    '../webkit/support/webkit_support.gyp:fileapi',
  ],
  'include_dirs': [
    '..',
  ],
  'export_dependent_settings': [
    '../base/base.gyp:base',
  ],
  'sources': [
    'public/common/bindings_policy.h',
    'public/common/child_process_sandbox_support_linux.h',
    'public/common/content_constants.cc',
    'public/common/content_constants.h',
    'public/common/content_paths.h',
    'public/common/content_restriction.h',
    'public/common/content_switches.cc',
    'public/common/content_switches.h',
    'public/common/dx_diag_node.cc',
    'public/common/dx_diag_node.h',
    'public/common/file_chooser_params.cc',
    'public/common/file_chooser_params.h',
    'public/common/frame_navigate_params.cc',
    'public/common/frame_navigate_params.h',
    'public/common/gpu_info.cc',
    'public/common/gpu_info.h',
    'public/common/main_function_params.h',
    'public/common/page_transition_types.cc',
    'public/common/page_transition_types.h',
    'public/common/page_type.h',
    'public/common/page_zoom.h',
    'public/common/pepper_plugin_info.cc',
    'public/common/pepper_plugin_info.h',
    'public/common/renderer_preferences.cc',
    'public/common/renderer_preferences.h',
    'public/common/resource_dispatcher_delegate.h',
    'public/common/result_codes.h',
    'public/common/sandbox_init.h',
    'public/common/security_style.h',
    'public/common/serialized_script_value.cc',
    'public/common/serialized_script_value.h',
    'public/common/show_desktop_notification_params.cc',
    'public/common/show_desktop_notification_params.h',
    'public/common/url_constants.cc',
    'public/common/url_constants.h',
    'public/common/url_fetcher.h',
    'public/common/url_fetcher_delegate.h',
    'public/common/url_fetcher_factory.h',
    'public/common/view_types.h',
    'public/common/zygote_fork_delegate_linux.h',
    'common/appcache/appcache_backend_proxy.cc',
    'common/appcache/appcache_backend_proxy.h',
    'common/appcache/appcache_dispatcher.cc',
    'common/appcache/appcache_dispatcher.h',
    'common/appcache_messages.h',
    'common/child_process.cc',
    'common/child_process.h',
    'common/child_process_host.cc',
    'common/child_process_host.h',
    'common/child_process_info.cc',
    'common/child_process_info.h',
    'common/child_process_messages.h',
    'common/child_process_sandbox_support_impl_linux.cc',
    'common/child_process_sandbox_support_impl_linux.h',
    'common/child_thread.cc',
    'common/child_thread.h',
    'common/child_trace_message_filter.cc',
    'common/child_trace_message_filter.h',
    'common/chrome_application_mac.h',
    'common/chrome_application_mac.mm',
    'common/chrome_descriptors.h',
    'common/clipboard_messages.h',
    'common/content_message_generator.cc',
    'common/content_message_generator.h',
    'common/content_export.h',
    'common/content_counters.cc',
    'common/content_counters.h',
    'common/content_paths.cc',
    'common/css_colors.h',
    'common/database_messages.h',
    'common/database_util.cc',
    'common/database_util.h',
    'common/db_message_filter.cc',
    'common/db_message_filter.h',
    'common/debug_flags.cc',
    'common/debug_flags.h',
    'common/desktop_notification_messages.h',
    'common/device_orientation_messages.h',
    'common/devtools_messages.h',
    'common/dom_storage_common.h',
    'common/dom_storage_messages.h',
    'common/drag_messages.h',
    'common/edit_command.h',
    'common/file_system/file_system_dispatcher.cc',
    'common/file_system/file_system_dispatcher.h',
    'common/file_system/webfilesystem_callback_dispatcher.cc',
    'common/file_system/webfilesystem_callback_dispatcher.h',
    'common/file_system/webfilesystem_impl.cc',
    'common/file_system/webfilesystem_impl.h',
    'common/file_system/webfilewriter_impl.cc',
    'common/file_system/webfilewriter_impl.h',
    'common/file_system_messages.h',
    'common/file_utilities_messages.h',
    'common/font_config_ipc_linux.cc',
    'common/font_config_ipc_linux.h',
    'common/font_list.h',
    'common/font_list_mac.mm',
    'common/font_list_win.cc',
    'common/font_list_x11.cc',
    'common/gamepad_hardware_buffer.h',
    'common/gamepad_messages.h',
    'common/geolocation_messages.h',
    'common/geoposition.cc',
    'common/geoposition.h',
    'common/gpu/gpu_channel.cc',
    'common/gpu/gpu_channel.h',
    'common/gpu/gpu_channel_manager.cc',
    'common/gpu/gpu_channel_manager.h',
    'common/gpu/gpu_command_buffer_stub.cc',
    'common/gpu/gpu_command_buffer_stub.h',
    'common/gpu/gpu_config.h',
    'common/gpu/gpu_feature_flags.cc',
    'common/gpu/gpu_feature_flags.h',
    'common/gpu/gpu_messages.h',
    'common/gpu/gpu_process_launch_causes.h',
    'common/gpu/gpu_watchdog.h',
    'common/gpu/image_transport_surface.h',
    'common/gpu/image_transport_surface.cc',
    'common/gpu/image_transport_surface_linux.cc',
    'common/gpu/image_transport_surface_mac.cc',
    'common/gpu/image_transport_surface_win.cc',
    'common/gpu/media/gpu_video_decode_accelerator.cc',
    'common/gpu/media/gpu_video_decode_accelerator.h',
    'common/gpu/transport_texture.cc',
    'common/gpu/transport_texture.h',
    'common/handle_enumerator_win.cc',
    'common/handle_enumerator_win.h',
    'common/hi_res_timer_manager_posix.cc',
    'common/hi_res_timer_manager_win.cc',
    'common/hi_res_timer_manager.h',
    'common/indexed_db_key.cc',
    'common/indexed_db_key.h',
    'common/indexed_db_messages.h',
    'common/indexed_db_param_traits.cc',
    'common/indexed_db_param_traits.h',
    'common/intents_messages.h',
    'common/java_bridge_messages.h',
    'common/mac/attributed_string_coder.h',
    'common/mac/attributed_string_coder.mm',
    'common/mac/font_descriptor.h',
    'common/mac/font_descriptor.mm',
    'common/mac/font_loader.h',
    'common/mac/font_loader.mm',
    'common/mac/scoped_sending_event.h',
    'common/mac/scoped_sending_event.mm',
    'common/media/audio_messages.h',
    'common/media/audio_stream_state.h',
    'common/media/media_stream_messages.h',
    'common/media/media_stream_options.cc',
    'common/media/media_stream_options.h',
    'common/media/video_capture_messages.h',
    'common/message_router.cc',
    'common/message_router.h',
    'common/mime_registry_messages.h',
    'common/navigation_gesture.h',
    'common/net/url_fetcher_impl.cc',
    'common/net/url_fetcher_impl.h',
    'common/np_channel_base.cc',
    'common/np_channel_base.h',
    'common/npobject_base.h',
    'common/npobject_proxy.cc',
    'common/npobject_proxy.h',
    'common/npobject_stub.cc',
    'common/npobject_stub.h',
    'common/npobject_util.cc',
    'common/npobject_util.h',
    'common/p2p_messages.h',
    'common/p2p_sockets.h',
    'common/page_zoom.cc',
    'common/pepper_file_messages.cc',
    'common/pepper_file_messages.h',
    'common/pepper_messages.h',
    'common/pepper_plugin_registry.cc',
    'common/pepper_plugin_registry.h',
    'common/plugin_carbon_interpose_constants_mac.cc',
    'common/plugin_carbon_interpose_constants_mac.h',
    'common/plugin_messages.h',
    'common/process_watcher.h',
    'common/process_watcher_mac.cc',
    'common/process_watcher_posix.cc',
    'common/process_watcher_win.cc',
    'common/quota_messages.h',
    'common/quota_dispatcher.cc',
    'common/quota_dispatcher.h',
    'common/request_extra_data.cc',
    'common/request_extra_data.h',
    'common/resource_dispatcher.cc',
    'common/resource_dispatcher.h',
    'common/resource_messages.h',
    'common/resource_response.cc',
    'common/resource_response.h',
    'common/sandbox_init_mac.cc',
    'common/sandbox_init_win.cc',
    'common/sandbox_mac.h',
    'common/sandbox_mac.mm',
    'common/sandbox_methods_linux.h',
    'common/sandbox_policy.cc',
    'common/sandbox_policy.h',
    'common/section_util_win.cc',
    'common/section_util_win.h',
    'common/sensors.h',
    'common/sensors_listener.h',
    'common/set_process_title.cc',
    'common/set_process_title.h',
    'common/set_process_title_linux.cc',
    'common/set_process_title_linux.h',
    'common/socket_stream.h',
    'common/socket_stream_dispatcher.cc',
    'common/socket_stream_dispatcher.h',
    'common/socket_stream_messages.h',
    'common/speech_input_messages.h',
    'common/speech_input_result.cc',
    'common/speech_input_result.h',
    'common/swapped_out_messages.cc',
    'common/swapped_out_messages.h',
    'common/text_input_client_messages.h',
    'common/unix_domain_socket_posix.cc',
    'common/unix_domain_socket_posix.h',
    'common/utility_messages.h',
    'common/view_messages.h',
    'common/view_message_enums.h',
    'common/web_database_observer_impl.cc',
    'common/web_database_observer_impl.h',
    'common/webblobregistry_impl.cc',
    'common/webblobregistry_impl.h',
    'common/webblob_messages.h',
    'common/webkitplatformsupport_impl.cc',
    'common/webkitplatformsupport_impl.h',
    'common/webmessageportchannel_impl.cc',
    'common/webmessageportchannel_impl.h',
    'common/worker_messages.h',
    'public/common/common_param_traits.cc',
    'public/common/common_param_traits.h',
    'public/common/content_client.cc',
    'public/common/content_client.h',
    'public/common/window_container_type.cc',
    'public/common/window_container_type.h',
    'public/common/webkit_param_traits.cc',
    'public/common/webkit_param_traits.h',
  ],
  'conditions': [
    ['OS!="win"', {
      'sources!': [
        'common/sandbox_policy.cc',
        'common/sandbox_policy.h',
      ],
    }],
    ['OS=="mac"', {
      'sources!': [
        'common/process_watcher_posix.cc',
      ],
      'link_settings': {
        'mac_bundle_resources': [
          'common/common.sb',
        ],
      },
    }],
    ['toolkit_uses_gtk == 1', {
      'dependencies': [
        '../build/linux/system.gyp:gtk',
      ],
    }],
    ['use_x11 == 1', {
      'dependencies': [
        '../build/linux/system.gyp:pangocairo',
      ],
    }],
    ['use_x11 == 1 and target_arch != "arm"', {
      'sources': [
        'common/gpu/x_util.cc',
        'common/gpu/x_util.h',
      ],
    }],
    ['OS=="linux"', {
      'include_dirs': [
        '<(DEPTH)/third_party/angle/include',
      ],
      'link_settings': {
        'libraries': [
          '-lXcomposite',
        ],
      },
    }],
    ['enable_gpu==1', {
      'dependencies': [
        '../gpu/gpu.gyp:command_buffer_service',
      ],
    }],
    ['target_arch=="arm"', {
      'dependencies': [
        '../media/media.gyp:media',
      ],
      'sources': [
        'common/gpu/media/gles2_texture_to_egl_image_translator.cc',
        'common/gpu/media/gles2_texture_to_egl_image_translator.h',
        'common/gpu/media/omx_video_decode_accelerator.cc',
        'common/gpu/media/omx_video_decode_accelerator.h',
      ],
      'include_dirs': [
        '<(DEPTH)/third_party/angle/include',
        '<(DEPTH)/third_party/openmax/il',
      ],
      'link_settings': {
        'libraries': [
          '-lEGL',
          '-lGLESv2',
        ],
      },
    }],
  ],
}
