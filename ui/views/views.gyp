# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
  'variables': {
    'chromium_code': 1,
    'external_ozone_views_files': [],
    # Sources lists shared with GN build.
    'views_sources': [
      'accessibility/native_view_accessibility.cc',
      'accessibility/native_view_accessibility.h',
      'accessibility/native_view_accessibility_win.cc',
      'accessibility/native_view_accessibility_win.h',
      'accessible_pane_view.cc',
      'accessible_pane_view.h',
      'animation/bounds_animator.cc',
      'animation/bounds_animator.h',
      'animation/scroll_animator.cc',
      'animation/scroll_animator.h',
      'background.cc',
      'background.h',
      'border.cc',
      'border.h',
      'bubble/bubble_border.cc',
      'bubble/bubble_border.h',
      'bubble/bubble_delegate.cc',
      'bubble/bubble_delegate.h',
      'bubble/bubble_frame_view.cc',
      'bubble/bubble_frame_view.h',
      'button_drag_utils.cc',
      'button_drag_utils.h',
      'cocoa/bridged_content_view.h',
      'cocoa/bridged_content_view.mm',
      'cocoa/bridged_native_widget.h',
      'cocoa/bridged_native_widget.mm',
      'cocoa/cocoa_mouse_capture.h',
      'cocoa/cocoa_mouse_capture.mm',
      'cocoa/cocoa_mouse_capture_delegate.h',
      'cocoa/native_widget_mac_nswindow.h',
      'cocoa/native_widget_mac_nswindow.mm',
      'cocoa/views_nswindow_delegate.h',
      'cocoa/views_nswindow_delegate.mm',
      'color_chooser/color_chooser_listener.h',
      'color_chooser/color_chooser_view.cc',
      'color_chooser/color_chooser_view.h',
      'color_constants.cc',
      'color_constants.h',
      'context_menu_controller.h',
      'controls/button/blue_button.cc',
      'controls/button/blue_button.h',
      'controls/button/button.cc',
      'controls/button/button.h',
      'controls/button/checkbox.cc',
      'controls/button/checkbox.h',
      'controls/button/custom_button.cc',
      'controls/button/custom_button.h',
      'controls/button/image_button.cc',
      'controls/button/image_button.h',
      'controls/button/label_button.cc',
      'controls/button/label_button.h',
      'controls/button/label_button_border.cc',
      'controls/button/label_button_border.h',
      'controls/button/menu_button.cc',
      'controls/button/menu_button.h',
      'controls/button/menu_button_listener.h',
      'controls/button/radio_button.cc',
      'controls/button/radio_button.h',
      'controls/combobox/combobox.cc',
      'controls/combobox/combobox.h',
      'controls/combobox/combobox_listener.h',
      'controls/focusable_border.cc',
      'controls/focusable_border.h',
      'controls/glow_hover_controller.cc',
      'controls/glow_hover_controller.h',
      'controls/image_view.cc',
      'controls/image_view.h',
      'controls/label.cc',
      'controls/label.h',
      'controls/link.cc',
      'controls/link.h',
      'controls/link_listener.h',
      'controls/menu/display_change_listener_mac.cc',
      'controls/menu/menu.cc',
      'controls/menu/menu.h',
      'controls/menu/menu_config.cc',
      'controls/menu/menu_config.h',
      'controls/menu/menu_config_mac.cc',
      'controls/menu/menu_config_win.cc',
      'controls/menu/menu_controller.cc',
      'controls/menu/menu_controller.h',
      'controls/menu/menu_controller_delegate.h',
      'controls/menu/menu_delegate.cc',
      'controls/menu/menu_delegate.h',
      'controls/menu/menu_event_dispatcher_linux.cc',
      'controls/menu/menu_event_dispatcher_linux.h',
      'controls/menu/menu_host.cc',
      'controls/menu/menu_host.h',
      'controls/menu/menu_host_root_view.cc',
      'controls/menu/menu_host_root_view.h',
      'controls/menu/menu_image_util.cc',
      'controls/menu/menu_image_util.h',
      'controls/menu/menu_insertion_delegate_win.h',
      'controls/menu/menu_item_view.cc',
      'controls/menu/menu_item_view.h',
      'controls/menu/menu_listener.cc',
      'controls/menu/menu_listener.h',
      'controls/menu/menu_message_loop.h',
      'controls/menu/menu_message_loop_mac.cc',
      'controls/menu/menu_message_loop_mac.h',
      'controls/menu/menu_message_pump_dispatcher_win.cc',
      'controls/menu/menu_message_pump_dispatcher_win.h',
      'controls/menu/menu_model_adapter.cc',
      'controls/menu/menu_model_adapter.h',
      'controls/menu/menu_runner.cc',
      'controls/menu/menu_runner.h',
      'controls/menu/menu_runner_handler.h',
      'controls/menu/menu_runner_impl.cc',
      'controls/menu/menu_runner_impl.h',
      'controls/menu/menu_runner_impl_adapter.cc',
      'controls/menu/menu_runner_impl_adapter.h',
      'controls/menu/menu_runner_impl_cocoa.h',
      'controls/menu/menu_runner_impl_cocoa.mm',
      'controls/menu/menu_runner_impl_interface.h',
      'controls/menu/menu_scroll_view_container.cc',
      'controls/menu/menu_scroll_view_container.h',
      'controls/menu/menu_separator.h',
      'controls/menu/menu_separator_views.cc',
      'controls/menu/menu_separator_win.cc',
      'controls/menu/menu_types.h',
      'controls/menu/native_menu_win.cc',
      'controls/menu/native_menu_win.h',
      'controls/menu/submenu_view.cc',
      'controls/menu/submenu_view.h',
      'controls/message_box_view.cc',
      'controls/message_box_view.h',
      'controls/native/native_view_host.cc',
      'controls/native/native_view_host.h',
      'controls/native/native_view_host_mac.h',
      'controls/native/native_view_host_mac.mm',
      'controls/prefix_delegate.h',
      'controls/prefix_selector.cc',
      'controls/prefix_selector.h',
      'controls/progress_bar.cc',
      'controls/progress_bar.h',
      'controls/resize_area.cc',
      'controls/resize_area.h',
      'controls/resize_area_delegate.h',
      'controls/scroll_view.cc',
      'controls/scroll_view.h',
      'controls/scrollbar/base_scroll_bar.cc',
      'controls/scrollbar/base_scroll_bar.h',
      'controls/scrollbar/base_scroll_bar_button.cc',
      'controls/scrollbar/base_scroll_bar_button.h',
      'controls/scrollbar/base_scroll_bar_thumb.cc',
      'controls/scrollbar/base_scroll_bar_thumb.h',
      'controls/scrollbar/kennedy_scroll_bar.cc',
      'controls/scrollbar/kennedy_scroll_bar.h',
      'controls/scrollbar/native_scroll_bar.cc',
      'controls/scrollbar/native_scroll_bar.h',
      'controls/scrollbar/native_scroll_bar_views.cc',
      'controls/scrollbar/native_scroll_bar_views.h',
      'controls/scrollbar/native_scroll_bar_wrapper.h',
      'controls/scrollbar/overlay_scroll_bar.cc',
      'controls/scrollbar/overlay_scroll_bar.h',
      'controls/scrollbar/scroll_bar.cc',
      'controls/scrollbar/scroll_bar.h',
      'controls/separator.cc',
      'controls/separator.h',
      'controls/single_split_view.cc',
      'controls/single_split_view.h',
      'controls/single_split_view_listener.h',
      'controls/slide_out_view.cc',
      'controls/slide_out_view.h',
      'controls/slider.cc',
      'controls/slider.h',
      'controls/styled_label.cc',
      'controls/styled_label.h',
      'controls/styled_label_listener.h',
      'controls/tabbed_pane/tabbed_pane.cc',
      'controls/tabbed_pane/tabbed_pane.h',
      'controls/tabbed_pane/tabbed_pane_listener.h',
      'controls/table/table_header.cc',
      'controls/table/table_header.h',
      'controls/table/table_utils.cc',
      'controls/table/table_utils.h',
      'controls/table/table_view.cc',
      'controls/table/table_view.h',
      'controls/table/table_view_observer.h',
      'controls/table/table_view_row_background_painter.h',
      'controls/textfield/textfield.cc',
      'controls/textfield/textfield.h',
      'controls/textfield/textfield_controller.cc',
      'controls/textfield/textfield_controller.h',
      'controls/textfield/textfield_model.cc',
      'controls/textfield/textfield_model.h',
      'controls/throbber.cc',
      'controls/throbber.h',
      'controls/tree/tree_view.cc',
      'controls/tree/tree_view.h',
      'controls/tree/tree_view_controller.cc',
      'controls/tree/tree_view_controller.h',
      'debug_utils.cc',
      'debug_utils.h',
      'drag_controller.h',
      'drag_utils.cc',
      'drag_utils.h',
      'drag_utils_mac.mm',
      'event_monitor.h',
      'event_monitor_mac.h',
      'event_monitor_mac.mm',
      'focus/external_focus_tracker.cc',
      'focus/external_focus_tracker.h',
      'focus/focus_manager.cc',
      'focus/focus_manager.h',
      'focus/focus_manager_delegate.h',
      'focus/focus_manager_factory.cc',
      'focus/focus_manager_factory.h',
      'focus/focus_search.cc',
      'focus/focus_search.h',
      'focus/view_storage.cc',
      'focus/view_storage.h',
      'focus/widget_focus_manager.cc',
      'focus/widget_focus_manager.h',
      'ime/input_method.h',
      'ime/input_method_base.cc',
      'ime/input_method_base.h',
      'ime/input_method_bridge.cc',
      'ime/input_method_bridge.h',
      'ime/input_method_delegate.h',
      'ime/mock_input_method.cc',
      'ime/mock_input_method.h',
      'ime/null_input_method.cc',
      'ime/null_input_method.h',
      'layout/box_layout.cc',
      'layout/box_layout.h',
      'layout/fill_layout.cc',
      'layout/fill_layout.h',
      'layout/grid_layout.cc',
      'layout/grid_layout.h',
      'layout/layout_constants.h',
      'layout/layout_manager.cc',
      'layout/layout_manager.h',
      'linux_ui/linux_ui.cc',
      'linux_ui/linux_ui.h',
      'linux_ui/status_icon_linux.cc',
      'linux_ui/status_icon_linux.h',
      'linux_ui/window_button_order_observer.h',
      'linux_ui/window_button_order_provider.cc',
      'masked_targeter_delegate.cc',
      'masked_targeter_delegate.h',
      'metrics.cc',
      'metrics.h',
      'metrics_mac.cc',
      'mouse_constants.h',
      'mouse_watcher.cc',
      'mouse_watcher.h',
      'mouse_watcher_view_host.cc',
      'mouse_watcher_view_host.h',
      'native_cursor.h',
      'native_cursor_mac.mm',
      'native_theme_delegate.h',
      'painter.cc',
      'painter.h',
      'rect_based_targeting_utils.cc',
      'rect_based_targeting_utils.h',
      'repeat_controller.cc',
      'repeat_controller.h',
      'round_rect_painter.cc',
      'round_rect_painter.h',
      'shadow_border.cc',
      'shadow_border.h',
      'view.cc',
      'view.h',
      'view_constants.cc',
      'view_constants.h',
      'view_model.cc',
      'view_model.h',
      'view_model_utils.cc',
      'view_model_utils.h',
      'view_targeter.cc',
      'view_targeter.h',
      'view_targeter_delegate.cc',
      'view_targeter_delegate.h',
      'views_delegate.cc',
      'views_delegate.h',
      'views_switches.cc',
      'views_switches.h',
      'views_touch_selection_controller_factory.h',
      'views_touch_selection_controller_factory_mac.cc',
      'widget/drop_helper.cc',
      'widget/drop_helper.h',
      'widget/monitor_win.cc',
      'widget/monitor_win.h',
      'widget/native_widget.h',
      'widget/native_widget_delegate.h',
      'widget/native_widget_mac.h',
      'widget/native_widget_mac.mm',
      'widget/native_widget_private.h',
      'widget/root_view.cc',
      'widget/root_view.h',
      'widget/root_view_targeter.cc',
      'widget/root_view_targeter.h',
      'widget/tooltip_manager.cc',
      'widget/tooltip_manager.h',
      'widget/widget.cc',
      'widget/widget.h',
      'widget/widget_aura_utils.cc',
      'widget/widget_aura_utils.h',
      'widget/widget_delegate.cc',
      'widget/widget_delegate.h',
      'widget/widget_deletion_observer.cc',
      'widget/widget_deletion_observer.h',
      'widget/widget_observer.h',
      'widget/widget_removals_observer.h',
      'window/client_view.cc',
      'window/client_view.h',
      'window/custom_frame_view.cc',
      'window/custom_frame_view.h',
      'window/dialog_client_view.cc',
      'window/dialog_client_view.h',
      'window/dialog_delegate.cc',
      'window/dialog_delegate.h',
      'window/frame_background.cc',
      'window/frame_background.h',
      'window/frame_buttons.h',
      'window/native_frame_view.cc',
      'window/native_frame_view.h',
      'window/non_client_view.cc',
      'window/non_client_view.h',
      'window/window_button_order_provider.cc',
      'window/window_button_order_provider.h',
      'window/window_resources.h',
      'window/window_shape.cc',
      'window/window_shape.h',
    ],
    'views_win_sources': [
      'controls/menu/menu_2.cc',
      'controls/menu/menu_2.h',
      'controls/menu/menu_wrapper.h',
      'widget/widget_hwnd_utils.cc',
      'widget/widget_hwnd_utils.h',
      'win/fullscreen_handler.cc',
      'win/fullscreen_handler.h',
      'win/hwnd_message_handler.cc',
      'win/hwnd_message_handler.h',
      'win/hwnd_message_handler_delegate.h',
      'win/hwnd_util.h',
      'win/hwnd_util_aurawin.cc',
      'win/scoped_fullscreen_visibility.cc',
      'win/scoped_fullscreen_visibility.h',
    ],
    'views_aura_sources': [
      'accessibility/ax_aura_obj_cache.cc',
      'accessibility/ax_aura_obj_cache.h',
      'accessibility/ax_view_obj_wrapper.cc',
      'accessibility/ax_view_obj_wrapper.h',
      'accessibility/ax_widget_obj_wrapper.cc',
      'accessibility/ax_widget_obj_wrapper.h',
      'accessibility/ax_window_obj_wrapper.cc',
      'accessibility/ax_window_obj_wrapper.h',
      'bubble/bubble_window_targeter.cc',
      'bubble/bubble_window_targeter.h',
      'bubble/tray_bubble_view.cc',
      'bubble/tray_bubble_view.h',
      'controls/menu/display_change_listener_aura.cc',
      'controls/menu/menu_config_aura.cc',
      'controls/menu/menu_message_loop_aura.cc',
      'controls/menu/menu_message_loop_aura.h',
      'controls/native/native_view_host_aura.cc',
      'controls/native/native_view_host_aura.h',
      'corewm/cursor_height_provider_win.cc',
      'corewm/cursor_height_provider_win.h',
      'corewm/tooltip.h',
      'corewm/tooltip_aura.cc',
      'corewm/tooltip_aura.h',
      'corewm/tooltip_controller.cc',
      'corewm/tooltip_controller.h',
      'corewm/tooltip_win.cc',
      'corewm/tooltip_win.h',
      'drag_utils_aura.cc',
      'event_monitor_aura.cc',
      'event_monitor_aura.h',
      'metrics_aura.cc',
      'native_cursor_aura.cc',
      'touchui/touch_editing_menu.cc',
      'touchui/touch_editing_menu.h',
      'touchui/touch_selection_controller_impl.cc',
      'touchui/touch_selection_controller_impl.h',
      'view_constants_aura.cc',
      'view_constants_aura.h',
      'views_touch_selection_controller_factory_aura.cc',
      'widget/native_widget_aura.cc',
      'widget/native_widget_aura.h',
      'widget/tooltip_manager_aura.cc',
      'widget/tooltip_manager_aura.h',
      'widget/window_reorderer.cc',
      'widget/window_reorderer.h',
    ],
    'views_desktop_aura_sources': [
      'widget/desktop_aura/desktop_capture_client.cc',
      'widget/desktop_aura/desktop_capture_client.h',
      'widget/desktop_aura/desktop_cursor_loader_updater.h',
      'widget/desktop_aura/desktop_dispatcher_client.cc',
      'widget/desktop_aura/desktop_dispatcher_client.h',
      'widget/desktop_aura/desktop_drop_target_win.cc',
      'widget/desktop_aura/desktop_drop_target_win.h',
      'widget/desktop_aura/desktop_event_client.cc',
      'widget/desktop_aura/desktop_event_client.h',
      'widget/desktop_aura/desktop_focus_rules.cc',
      'widget/desktop_aura/desktop_focus_rules.h',
      'widget/desktop_aura/desktop_native_cursor_manager.cc',
      'widget/desktop_aura/desktop_native_cursor_manager.h',
      'widget/desktop_aura/desktop_native_widget_aura.cc',
      'widget/desktop_aura/desktop_native_widget_aura.h',
      'widget/desktop_aura/desktop_screen.h',
      'widget/desktop_aura/desktop_screen_position_client.cc',
      'widget/desktop_aura/desktop_screen_position_client.h',
      'widget/desktop_aura/desktop_window_tree_host.h',
    ],
    'views_desktop_aura_linux_sources': [
      'widget/desktop_aura/desktop_cursor_loader_updater_auralinux.cc',
      'widget/desktop_aura/desktop_cursor_loader_updater_auralinux.h',
    ],
    'views_desktop_aura_x11_sources': [
      'accessibility/native_view_accessibility_auralinux.cc',
      'accessibility/native_view_accessibility_auralinux.h',
      'widget/desktop_aura/desktop_drag_drop_client_aurax11.cc',
      'widget/desktop_aura/desktop_drag_drop_client_aurax11.h',
      'widget/desktop_aura/desktop_screen_x11.cc',
      'widget/desktop_aura/desktop_screen_x11.h',
      'widget/desktop_aura/desktop_window_tree_host_x11.cc',
      'widget/desktop_aura/desktop_window_tree_host_x11.h',
      'widget/desktop_aura/x11_desktop_handler.cc',
      'widget/desktop_aura/x11_desktop_handler.h',
      'widget/desktop_aura/x11_desktop_window_move_client.cc',
      'widget/desktop_aura/x11_desktop_window_move_client.h',
      'widget/desktop_aura/x11_move_loop.h',
      'widget/desktop_aura/x11_move_loop_delegate.h',
      'widget/desktop_aura/x11_pointer_grab.cc',
      'widget/desktop_aura/x11_pointer_grab.h',
      'widget/desktop_aura/x11_topmost_window_finder.cc',
      'widget/desktop_aura/x11_topmost_window_finder.h',
      'widget/desktop_aura/x11_whole_screen_move_loop.cc',
      'widget/desktop_aura/x11_whole_screen_move_loop.h',
      'widget/desktop_aura/x11_window_event_filter.cc',
      'widget/desktop_aura/x11_window_event_filter.h',
    ],
    'views_desktop_aura_win_sources': [
      'widget/desktop_aura/desktop_cursor_loader_updater_aurawin.cc',
      'widget/desktop_aura/desktop_drag_drop_client_win.cc',
      'widget/desktop_aura/desktop_drag_drop_client_win.h',
      'widget/desktop_aura/desktop_screen_win.cc',
      'widget/desktop_aura/desktop_screen_win.h',
      'widget/desktop_aura/desktop_window_tree_host_win.cc',
      'widget/desktop_aura/desktop_window_tree_host_win.h',
    ],
    'views_test_support_sources': [
      'controls/textfield/textfield_test_api.cc',
      'controls/textfield/textfield_test_api.h',
      'test/capture_tracking_view.cc',
      'test/capture_tracking_view.h',
      'test/desktop_test_views_delegate.h',
      'test/desktop_test_views_delegate_mac.mm',
      'test/event_generator_delegate_mac.h',
      'test/event_generator_delegate_mac.mm',
      'test/focus_manager_test.cc',
      'test/focus_manager_test.h',
      'test/menu_runner_test_api.cc',
      'test/menu_runner_test_api.h',
      'test/slider_test_api.cc',
      'test/slider_test_api.h',
      'test/test_views.cc',
      'test/test_views.h',
      'test/test_views_delegate.h',
      'test/test_views_delegate_mac.mm',
      'test/test_widget_observer.cc',
      'test/test_widget_observer.h',
      'test/views_test_base.cc',
      'test/views_test_base.h',
      'test/views_test_helper.cc',
      'test/views_test_helper.h',
      'test/views_test_helper_mac.h',
      'test/views_test_helper_mac.mm',
      'test/widget_test.cc',
      'test/widget_test.h',
      'test/widget_test_mac.mm',
      'test/x11_property_change_waiter.cc',
      'test/x11_property_change_waiter.h',
    ],
    'views_test_support_aura_sources': [
      'corewm/tooltip_controller_test_helper.cc',
      'corewm/tooltip_controller_test_helper.h',
      'test/desktop_test_views_delegate_aura.cc',
      'test/test_views_delegate_aura.cc',
      'test/views_test_helper_aura.cc',
      'test/views_test_helper_aura.h',
      'test/widget_test_aura.cc',
    ],
    'views_test_support_desktop_aura_x11_sources': [
      'test/ui_controls_factory_desktop_aurax11.cc',
      'test/ui_controls_factory_desktop_aurax11.h',
    ],
    'views_unittests_sources': [
      'accessibility/native_view_accessibility_unittest.cc',
      'accessibility/native_view_accessibility_win_unittest.cc',
      'accessible_pane_view_unittest.cc',
      'animation/bounds_animator_unittest.cc',
      'bubble/bubble_border_unittest.cc',
      'bubble/bubble_delegate_unittest.cc',
      'bubble/bubble_frame_view_unittest.cc',
      'bubble/bubble_window_targeter_unittest.cc',
      'cocoa/bridged_native_widget_unittest.mm',
      'cocoa/cocoa_mouse_capture_unittest.mm',
      'controls/button/blue_button_unittest.cc',
      'controls/button/custom_button_unittest.cc',
      'controls/button/image_button_unittest.cc',
      'controls/button/label_button_unittest.cc',
      'controls/button/menu_button_unittest.cc',
      'controls/combobox/combobox_unittest.cc',
      'controls/label_unittest.cc',
      'controls/menu/menu_controller_unittest.cc',
      'controls/menu/menu_item_view_unittest.cc',
      'controls/menu/menu_model_adapter_unittest.cc',
      'controls/menu/menu_runner_cocoa_unittest.mm',
      'controls/native/native_view_host_mac_unittest.mm',
      'controls/native/native_view_host_test_base.cc',
      'controls/native/native_view_host_test_base.h',
      'controls/native/native_view_host_unittest.cc',
      'controls/prefix_selector_unittest.cc',
      'controls/progress_bar_unittest.cc',
      'controls/scroll_view_unittest.cc',
      'controls/scrollbar/scrollbar_unittest.cc',
      'controls/single_split_view_unittest.cc',
      'controls/slider_unittest.cc',
      'controls/styled_label_unittest.cc',
      'controls/tabbed_pane/tabbed_pane_unittest.cc',
      'controls/table/table_utils_unittest.cc',
      'controls/table/table_view_unittest.cc',
      'controls/table/test_table_model.cc',
      'controls/table/test_table_model.h',
      'controls/textfield/textfield_model_unittest.cc',
      'controls/textfield/textfield_unittest.cc',
      'controls/tree/tree_view_unittest.cc',
      'event_monitor_unittest.cc',
      'focus/focus_manager_unittest.cc',
      'focus/focus_traversal_unittest.cc',
      'layout/box_layout_unittest.cc',
      'layout/grid_layout_unittest.cc',
      'rect_based_targeting_utils_unittest.cc',
      'run_all_unittests.cc',
      'view_model_unittest.cc',
      'view_model_utils_unittest.cc',
      'view_targeter_unittest.cc',
      'view_unittest.cc',
      'widget/native_widget_mac_unittest.mm',
      'widget/native_widget_unittest.cc',
      'widget/root_view_unittest.cc',
      'widget/widget_unittest.cc',
      'widget/window_reorderer_unittest.cc',
      'window/custom_frame_view_unittest.cc',
      'window/dialog_client_view_unittest.cc',
      'window/dialog_delegate_unittest.cc',
    ],
    'views_unittests_desktop_sources': [
      'ime/input_method_bridge_unittest.cc',
      'widget/desktop_widget_unittest.cc',
    ],
    'views_unittests_aura_sources': [
      'controls/native/native_view_host_aura_unittest.cc',
      'corewm/tooltip_controller_unittest.cc',
      'touchui/touch_selection_controller_impl_unittest.cc',
      'view_unittest_aura.cc',
      'widget/native_widget_aura_unittest.cc',
    ],
    'views_unittests_desktop_aura_sources': [
      'widget/desktop_aura/desktop_focus_rules_unittest.cc',
      'widget/desktop_aura/desktop_native_widget_aura_unittest.cc',
    ],
    'views_unittests_desktop_aurax11_sources': [
      'widget/desktop_aura/desktop_drag_drop_client_aurax11_unittest.cc',
      'widget/desktop_aura/desktop_screen_x11_unittest.cc',
      'widget/desktop_aura/desktop_window_tree_host_x11_unittest.cc',
    ],
  },
  'targets': [
    {
      # GN version: //ui/views
      'target_name': 'views',
      'type': '<(component)',
      'dependencies': [
        '../../base/base.gyp:base',
        '../../base/base.gyp:base_i18n',
        '../../base/third_party/dynamic_annotations/dynamic_annotations.gyp:dynamic_annotations',
        '../../skia/skia.gyp:skia',
        '../../third_party/icu/icu.gyp:icui18n',
        '../../third_party/icu/icu.gyp:icuuc',
        '../../url/url.gyp:url_lib',
        '../accessibility/accessibility.gyp:accessibility',
        '../accessibility/accessibility.gyp:ax_gen',
        '../base/ime/ui_base_ime.gyp:ui_base_ime',
        '../base/ui_base.gyp:ui_base',
        '../compositor/compositor.gyp:compositor',
        '../events/events.gyp:events',
        '../events/events.gyp:events_base',
        '../events/platform/events_platform.gyp:events_platform',
        '../gfx/gfx.gyp:gfx',
        '../gfx/gfx.gyp:gfx_geometry',
        '../native_theme/native_theme.gyp:native_theme',
        '../resources/ui_resources.gyp:ui_resources',
        '../strings/ui_strings.gyp:ui_strings',
      ],
      'export_dependent_settings': [
        '../accessibility/accessibility.gyp:ax_gen',
      ],
      'defines': [
        'VIEWS_IMPLEMENTATION',
      ],
      'sources': [
        '<@(views_sources)',
      ],
      'conditions': [
        ['use_ash==0', {
          'sources!': [
            'bubble/tray_bubble_view.cc',
            'bubble/tray_bubble_view.h',
          ],
        }],
        ['chromeos==0 and use_x11==1', {
          'dependencies': [
            '../display/display.gyp:display_util',
          ],
        }],
        ['OS=="linux" and chromeos==0 and use_ozone==0', {
          'dependencies': [
            '../../build/linux/system.gyp:atk',
           ],
        }],
        ['OS=="linux" and chromeos==0', {
          'dependencies': [
            '../shell_dialogs/shell_dialogs.gyp:shell_dialogs',
          ],
          'sources!': [
            'window/window_button_order_provider.cc',
          ],
        }, { # OS=="linux" and chromeos==0
          'sources/': [
            ['exclude', 'linux_ui'],
          ],
        }],
        ['OS=="win"', {
          'sources': [
            '<@(views_win_sources)',
          ],
          'dependencies': [
            # For accessibility
            '../../third_party/iaccessible2/iaccessible2.gyp:iaccessible2',
          ],
          'include_dirs': [
            '../../third_party/wtl/include',
          ],
          'link_settings': {
            'libraries': [
              '-limm32.lib',
              '-loleacc.lib',
            ],
            'msvs_settings': {
              'VCLinkerTool': {
                'DelayLoadDLLs': [
                  'user32.dll',
                ],
              },
            },
          },
          # TODO(jschuh): crbug.com/167187 fix size_t to int truncations.
          'msvs_disabled_warnings': [ 4267, ],
        }],
        ['use_ozone==1', {
          'sources': [
            '<@(external_ozone_views_files)',
          ],
          'dependencies': [
            '../ozone/ozone.gyp:ozone',
          ],
        }],
        ['use_x11==1', {
          'dependencies': [
            '../../build/linux/system.gyp:x11',
            '../../build/linux/system.gyp:xrandr',
            '../events/devices/events_devices.gyp:events_devices',
            '../events/platform/x11/x11_events_platform.gyp:x11_events_platform',
            '../gfx/x/gfx_x11.gyp:gfx_x11',
          ],
        }],
        ['use_aura==1', {
          'sources': [
            '<@(views_aura_sources)',
          ],
          'dependencies': [
            '../aura/aura.gyp:aura',
            '../wm/wm.gyp:wm',
          ],
        }],
        ['use_aura and chromeos == 0', {
          'sources': [ '<@(views_desktop_aura_sources)' ],
          'conditions': [
            ['OS == "linux"', {
              'sources': [ '<@(views_desktop_aura_linux_sources)' ],
            }],
            ['use_x11 == 1', {
              'sources': [ '<@(views_desktop_aura_x11_sources)' ],
              'dependencies': [
                '../../build/linux/system.gyp:xext',
              ],
            }],
            ['OS == "win"', {
              'sources': [ '<@(views_desktop_aura_win_sources)' ],
            }],
          ],
        }],
        ['OS=="mac"', {
          'dependencies': [
            '../accelerated_widget_mac/accelerated_widget_mac.gyp:accelerated_widget_mac',
          ],
        }],
      ],
    }, # target_name: views
    {
      # GN version: //ui/views:test_support
      'target_name': 'views_test_support',
      'type': 'static_library',
      'dependencies': [
        '../../base/base.gyp:base',
        '../../ipc/ipc.gyp:test_support_ipc',
        '../../skia/skia.gyp:skia',
        '../../testing/gtest.gyp:gtest',
        '../base/ime/ui_base_ime.gyp:ui_base_ime',
        '../base/ui_base.gyp:ui_base',
        '../compositor/compositor.gyp:compositor',
        '../compositor/compositor.gyp:compositor_test_support',
        '../events/events.gyp:events',
        '../events/events.gyp:events_test_support',
        '../events/platform/events_platform.gyp:events_platform',
        '../gfx/gfx.gyp:gfx',
        '../gfx/gfx.gyp:gfx_geometry',
        'views',
      ],
      'include_dirs': [
        '..',
      ],
      'sources': [
        '<@(views_test_support_sources)',
      ],
      'conditions': [
        ['use_aura==1', {
          'sources': [ '<@(views_test_support_aura_sources)' ],
          'dependencies': [
            '../aura/aura.gyp:aura',
            '../aura/aura.gyp:aura_test_support',
            '../wm/wm.gyp:wm',
          ],
        }],
        ['use_aura==1 and use_x11==1 and chromeos==0', {
          'sources': [ '<@(views_test_support_desktop_aura_x11_sources)' ],
        }],
      ],
    },  # target_name: views_test_support
    {
      # GN version: //ui/views:views_unittests
      'target_name': 'views_unittests',
      'type': 'executable',
      'dependencies': [
        '../../base/base.gyp:base',
        '../../base/base.gyp:base_i18n',
        '../../base/base.gyp:test_support_base',
        '../../skia/skia.gyp:skia',
        '../../testing/gtest.gyp:gtest',
        '../../third_party/icu/icu.gyp:icui18n',
        '../../third_party/icu/icu.gyp:icuuc',
        '../../url/url.gyp:url_lib',
        '../accessibility/accessibility.gyp:accessibility',
        '../base/ime/ui_base_ime.gyp:ui_base_ime',
        '../base/ui_base.gyp:ui_base',
        '../base/ui_base.gyp:ui_base_test_support',
        '../compositor/compositor.gyp:compositor',
        '../events/events.gyp:events',
        '../events/events.gyp:events_base',
        '../events/events.gyp:events_test_support',
        '../gfx/gfx.gyp:gfx',
        '../gfx/gfx.gyp:gfx_geometry',
        '../resources/ui_resources.gyp:ui_resources',
        '../resources/ui_resources.gyp:ui_test_pak',
        '../strings/ui_strings.gyp:ui_strings',
        'views',
        'views_test_support',
      ],
      'include_dirs': [
        '..',
      ],
      'sources': [
        '<@(views_unittests_sources)',
      ],
      'conditions': [
        ['OS=="win"', {
          'dependencies': [
            '../../third_party/iaccessible2/iaccessible2.gyp:iaccessible2',
          ],
          'link_settings': {
            'libraries': [
              '-limm32.lib',
              '-loleacc.lib',
              '-lcomctl32.lib',
            ]
          },
          'include_dirs': [
            '../third_party/wtl/include',
          ],
          'msvs_settings': {
            'VCManifestTool': {
              'AdditionalManifestFiles': [
                '$(ProjectDir)\\test\\views_unittest.manifest',
              ],
            },
          },
        }],
        ['OS=="win" and win_use_allocator_shim==1', {
          'dependencies': [
            '../../base/allocator/allocator.gyp:allocator',
          ],
        }],
        ['OS=="linux" and use_allocator!="none"', {
           # See http://crbug.com/162998#c4 for why this is needed.
          'dependencies': [
            '../../base/allocator/allocator.gyp:allocator',
          ],
        }],
        ['use_x11==1', {
          'dependencies': [
            '../../build/linux/system.gyp:x11',
            '../../build/linux/system.gyp:xext',
            '../events/devices/events_devices.gyp:events_devices',
            '../events/platform/x11/x11_events_platform.gyp:x11_events_platform',
          ],
        }],
        ['use_aura==1', {
          'sources': [ '<@(views_unittests_aura_sources)' ],
          'dependencies': [
            '../aura/aura.gyp:aura',
            '../aura/aura.gyp:aura_test_support',
            '../wm/wm.gyp:wm',
          ],
          'conditions': [
            ['chromeos == 0', {
              'sources': [ '<@(views_unittests_desktop_aura_sources)' ],
            }],
            ['chromeos == 0 and use_x11==1', {
              'sources': [ '<@(views_unittests_desktop_aurax11_sources)' ],
            }],
          ]
        }],
        ['chromeos==0', {
          'sources': [ '<@(views_unittests_desktop_sources)' ],
        }],
        ['use_x11==1', {
          'dependencies': [
            '../events/platform/x11/x11_events_platform.gyp:x11_events_platform',
          ],
        }],
        ['OS=="mac"', {
          # views_unittests not yet compiling on Mac. http://crbug.com/378134
          'sources!': [
            'bubble/bubble_window_targeter_unittest.cc',
            'controls/button/custom_button_unittest.cc',
            'controls/menu/menu_controller_unittest.cc',
            'controls/native/native_view_host_unittest.cc',
            'focus/focus_manager_unittest.cc',
            'ime/input_method_bridge_unittest.cc',
            'widget/window_reorderer_unittest.cc',
          ]
        }],
      ],
    },  # target_name: views_unittests
  ],  # targets
  'conditions': [
    ['OS=="mac"', {
      'targets': [
        {
          # GN version: //ui/views:macviews_interactive_ui_tests
          'target_name': 'macviews_interactive_ui_tests',
          'type': 'executable',
          'dependencies': [
            '../../base/base.gyp:base',
            '../../base/base.gyp:test_support_base',
            '../../skia/skia.gyp:skia',
            '../../testing/gtest.gyp:gtest',
            '../compositor/compositor.gyp:compositor_test_support',
            '../resources/ui_resources.gyp:ui_resources',
            '../resources/ui_resources.gyp:ui_test_pak',
            '../strings/ui_strings.gyp:ui_strings',
            'views',
            'views_test_support',
          ],
          'sources': [
            'cocoa/bridged_native_widget_interactive_uitest.mm',
            'run_all_unittests.cc',
            'widget/native_widget_mac_interactive_uitest.mm',
          ],
          'conditions': [
            ['use_aura == 1', {
              'dependencies': [
                '../aura/aura.gyp:aura',
                '../wm/wm.gyp:wm',
              ],
            }],
          ],
        },  # target_name: macviews_interactive_ui_tests
      ],  # targets
    }],
  ],  # conditions
}
