// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/views/apps/app_info_dialog/app_info_permissions_panel.h"

#include <string>
#include <vector>

#include "apps/app_load_service.h"
#include "apps/saved_files_service.h"
#include "base/files/file_path.h"
#include "base/strings/string_split.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/grit/generated_resources.h"
#include "extensions/browser/api/device_permissions_manager.h"
#include "extensions/common/extension.h"
#include "extensions/common/permissions/api_permission.h"
#include "extensions/common/permissions/permissions_data.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/resources/grit/ui_resources.h"
#include "ui/views/border.h"
#include "ui/views/controls/button/button.h"
#include "ui/views/controls/button/image_button.h"
#include "ui/views/controls/label.h"
#include "ui/views/layout/box_layout.h"
#include "ui/views/layout/grid_layout.h"
#include "ui/views/layout/layout_constants.h"
#include "ui/views/view.h"

namespace {

// IDs for the two bullet column sets.
const int kBulletColumnSetId = 1;
const int kNestedBulletColumnSetId = 2;

// Pixel spacing measurements for different parts of the permissions list.
const int kSpacingBetweenBulletAndStartOfText = 5;
const int kSpacingBetweenTextAndRevokeButton = 15;
const int kIndentationBeforeNestedBullet = 13;

// Creates a close button that calls |callback| on click and can be placed to
// the right of a bullet in the permissions list. The alt-text is set to a
// revoke message containing the given |permission_message|.
class RevokeButton : public views::ImageButton, public views::ButtonListener {
 public:
  explicit RevokeButton(const base::Closure& callback,
                        base::string16 permission_message)
      : views::ImageButton(this), callback_(callback) {
    ui::ResourceBundle& rb = ui::ResourceBundle::GetSharedInstance();
    SetImage(views::CustomButton::STATE_NORMAL,
             rb.GetImageNamed(IDR_DISABLE).ToImageSkia());
    SetImage(views::CustomButton::STATE_HOVERED,
             rb.GetImageNamed(IDR_DISABLE_H).ToImageSkia());
    SetImage(views::CustomButton::STATE_PRESSED,
             rb.GetImageNamed(IDR_DISABLE_P).ToImageSkia());
    SetBorder(scoped_ptr<views::Border>());
    SetSize(GetPreferredSize());

    // Make the button focusable & give it alt-text so permissions can be
    // revoked using only the keyboard.
    SetFocusable(true);
    SetTooltipText(l10n_util::GetStringFUTF16(
        IDS_APPLICATION_INFO_REVOKE_PERMISSION_ALT_TEXT, permission_message));
  }
  ~RevokeButton() override {}

 private:
  // Overridden from views::ButtonListener.
  void ButtonPressed(views::Button* sender, const ui::Event& event) override {
    DCHECK_EQ(this, sender);
    if (!callback_.is_null())
      callback_.Run();
  }

  const base::Closure callback_;

  DISALLOW_COPY_AND_ASSIGN(RevokeButton);
};

// A bulleted list of permissions.
// TODO(sashab): Fix BoxLayout to correctly display multi-line strings and then
// remove this class (since the GridLayout will no longer be needed).
class BulletedPermissionsList : public views::View {
 public:
  BulletedPermissionsList() {
    layout_ = new views::GridLayout(this);
    SetLayoutManager(layout_);

    // Create 3 columns: the bullet, the bullet text, and the revoke button.
    views::ColumnSet* column_set = layout_->AddColumnSet(kBulletColumnSetId);
    column_set->AddColumn(views::GridLayout::FILL,
                          views::GridLayout::LEADING,
                          0,
                          views::GridLayout::USE_PREF,
                          0,
                          0);
    column_set->AddPaddingColumn(0, kSpacingBetweenBulletAndStartOfText);
    column_set->AddColumn(views::GridLayout::FILL,
                          views::GridLayout::LEADING,
                          1 /* stretch to fill space */,
                          views::GridLayout::USE_PREF,
                          0,
                          0);
    column_set->AddPaddingColumn(0, kSpacingBetweenTextAndRevokeButton);
    column_set->AddColumn(views::GridLayout::FILL,
                          views::GridLayout::LEADING,
                          0,
                          views::GridLayout::USE_PREF,
                          0,
                          0);

    views::ColumnSet* nested_column_set =
        layout_->AddColumnSet(kNestedBulletColumnSetId);
    nested_column_set->AddPaddingColumn(0, kIndentationBeforeNestedBullet);
    nested_column_set->AddColumn(views::GridLayout::FILL,
                                 views::GridLayout::LEADING,
                                 0,
                                 views::GridLayout::USE_PREF,
                                 0,
                                 0);
    nested_column_set->AddPaddingColumn(0, kSpacingBetweenBulletAndStartOfText);
    nested_column_set->AddColumn(views::GridLayout::FILL,
                                 views::GridLayout::LEADING,
                                 1 /* stretch to fill space */,
                                 views::GridLayout::USE_PREF,
                                 0,
                                 0);
    nested_column_set->AddPaddingColumn(0, kSpacingBetweenTextAndRevokeButton);
    nested_column_set->AddColumn(views::GridLayout::FILL,
                                 views::GridLayout::LEADING,
                                 0,
                                 views::GridLayout::USE_PREF,
                                 0,
                                 0);
  }
  ~BulletedPermissionsList() override {}

  // Given a set of strings for a given permission (|message| for the topmost
  // bullet and a potentially-empty |submessages| for sub-bullets), adds these
  // bullets to the given BulletedPermissionsList. If |revoke_callback| is
  // provided, also adds an X button next to the bullet which calls the callback
  // when clicked.
  void AddPermissionBullets(base::string16 message,
                            std::vector<base::string16> submessages,
                            gfx::ElideBehavior elide_behavior_for_submessages,
                            const base::Closure& revoke_callback) {
    RevokeButton* revoke_button = NULL;
    if (!revoke_callback.is_null())
      revoke_button = new RevokeButton(revoke_callback, message);

    views::Label* permission_label = new views::Label(message);
    permission_label->SetHorizontalAlignment(gfx::ALIGN_LEFT);
    permission_label->SetMultiLine(true);
    AddSinglePermissionBullet(false, permission_label, revoke_button);

    for (const auto& submessage : submessages) {
      views::Label* sub_permission_label = new views::Label(submessage);
      sub_permission_label->SetHorizontalAlignment(gfx::ALIGN_LEFT);
      sub_permission_label->SetElideBehavior(elide_behavior_for_submessages);
      AddSinglePermissionBullet(true, sub_permission_label, NULL);
    }
  }

 private:
  void AddSinglePermissionBullet(bool is_nested,
                                 views::Label* permission_label,
                                 RevokeButton* revoke_button) {
    // Add a padding row before every item except the first.
    if (has_children())
      layout_->AddPaddingRow(0, views::kRelatedControlVerticalSpacing);

    const base::char16 bullet_point[] = {0x2022, 0};
    views::Label* bullet_label = new views::Label(base::string16(bullet_point));

    layout_->StartRow(
        1, is_nested ? kNestedBulletColumnSetId : kBulletColumnSetId);
    layout_->AddView(bullet_label);
    layout_->AddView(permission_label);

    if (revoke_button != NULL)
      layout_->AddView(revoke_button);
    else
      layout_->SkipColumns(1);
  }

  views::GridLayout* layout_;

  DISALLOW_COPY_AND_ASSIGN(BulletedPermissionsList);
};

}  // namespace

AppInfoPermissionsPanel::AppInfoPermissionsPanel(
    Profile* profile,
    const extensions::Extension* app)
    : AppInfoPanel(profile, app) {
  SetLayoutManager(new views::BoxLayout(views::BoxLayout::kVertical,
                                        0,
                                        0,
                                        views::kRelatedControlVerticalSpacing));

  CreatePermissionsList();
}

AppInfoPermissionsPanel::~AppInfoPermissionsPanel() {
}

void AppInfoPermissionsPanel::CreatePermissionsList() {
  views::View* permissions_heading = CreateHeading(
      l10n_util::GetStringUTF16(IDS_APPLICATION_INFO_APP_PERMISSIONS_TITLE));
  AddChildView(permissions_heading);

  if (!HasActivePermissionMessages() && GetRetainedDeviceCount() == 0 &&
      GetRetainedFileCount() == 0) {
    views::Label* no_permissions_text =
        new views::Label(l10n_util::GetStringUTF16(
            app_->is_extension()
                ? IDS_APPLICATION_INFO_EXTENSION_NO_PERMISSIONS_TEXT
                : IDS_APPLICATION_INFO_APP_NO_PERMISSIONS_TEXT));
    no_permissions_text->SetHorizontalAlignment(gfx::ALIGN_LEFT);
    AddChildView(no_permissions_text);
    return;
  }

  BulletedPermissionsList* permissions_list = new BulletedPermissionsList();

  // Add regular and host permission messages.
  for (const auto& message : GetActivePermissionMessages()) {
    permissions_list->AddPermissionBullets(
        message.first, message.second, gfx::ELIDE_MIDDLE, base::Closure());
  }

  // Add USB devices, if the app has any.
  if (GetRetainedDeviceCount() > 0) {
    permissions_list->AddPermissionBullets(
        GetRetainedDeviceHeading(),
        GetRetainedDevices(),
        gfx::ELIDE_TAIL,
        base::Bind(&AppInfoPermissionsPanel::RevokeDevicePermissions,
                   base::Unretained(this)));
  }

  // Add retained files, if the app has any.
  if (GetRetainedFileCount() > 0) {
    permissions_list->AddPermissionBullets(
        GetRetainedFileHeading(),
        GetRetainedFilePaths(),
        gfx::ELIDE_MIDDLE,
        base::Bind(&AppInfoPermissionsPanel::RevokeFilePermissions,
                   base::Unretained(this)));
  }

  AddChildView(permissions_list);
}

bool AppInfoPermissionsPanel::HasActivePermissionMessages() const {
  return !GetActivePermissionMessages().empty();
}

const std::vector<PermissionStringAndDetailsPair>
AppInfoPermissionsPanel::GetActivePermissionMessages() const {
  std::vector<PermissionStringAndDetailsPair> messages_with_details;
  std::vector<base::string16> permission_messages =
      app_->permissions_data()->GetLegacyPermissionMessageStrings();
  std::vector<base::string16> permission_message_details =
      app_->permissions_data()->GetLegacyPermissionMessageDetailsStrings();
  DCHECK_EQ(permission_messages.size(), permission_message_details.size());

  for (size_t i = 0; i < permission_messages.size(); i++) {
    std::vector<base::string16> details;
    if (!permission_message_details[i].empty()) {
      // Make each new line in the details a separate sub-bullet.
      base::SplitString(
          permission_message_details[i], base::char16('\n'), &details);
    }
    messages_with_details.push_back(
        PermissionStringAndDetailsPair(permission_messages[i], details));
  }
  return messages_with_details;
}

int AppInfoPermissionsPanel::GetRetainedFileCount() const {
  if (app_->permissions_data()->HasAPIPermission(
          extensions::APIPermission::kFileSystem)) {
    apps::SavedFilesService* service = apps::SavedFilesService::Get(profile_);
    // The SavedFilesService can be null for incognito profiles. See
    // http://crbug.com/467795.
    if (service)
      return service->GetAllFileEntries(app_->id()).size();
  }
  return 0;
}

base::string16 AppInfoPermissionsPanel::GetRetainedFileHeading() const {
  const int kRetainedFilesMessageIDs[6] = {
      IDS_APPLICATION_INFO_RETAINED_FILES_DEFAULT,
      IDS_APPLICATION_INFO_RETAINED_FILE_SINGULAR,
      IDS_APPLICATION_INFO_RETAINED_FILES_ZERO,
      IDS_APPLICATION_INFO_RETAINED_FILES_TWO,
      IDS_APPLICATION_INFO_RETAINED_FILES_FEW,
      IDS_APPLICATION_INFO_RETAINED_FILES_MANY,
  };
  std::vector<int> message_ids(
      kRetainedFilesMessageIDs,
      kRetainedFilesMessageIDs + arraysize(kRetainedFilesMessageIDs));

  return l10n_util::GetPluralStringFUTF16(message_ids, GetRetainedFileCount());
}

const std::vector<base::string16>
AppInfoPermissionsPanel::GetRetainedFilePaths() const {
  std::vector<base::string16> retained_file_paths;
  if (app_->permissions_data()->HasAPIPermission(
          extensions::APIPermission::kFileSystem)) {
    apps::SavedFilesService* service = apps::SavedFilesService::Get(profile_);
    // The SavedFilesService can be null for incognito profiles.
    if (service) {
      std::vector<apps::SavedFileEntry> retained_file_entries =
          service->GetAllFileEntries(app_->id());
      for (std::vector<apps::SavedFileEntry>::const_iterator it =
               retained_file_entries.begin();
           it != retained_file_entries.end(); ++it) {
        retained_file_paths.push_back(it->path.LossyDisplayName());
      }
    }
  }
  return retained_file_paths;
}

void AppInfoPermissionsPanel::RevokeFilePermissions() {
  apps::SavedFilesService* service = apps::SavedFilesService::Get(profile_);
  // The SavedFilesService can be null for incognito profiles.
  if (service)
    service->ClearQueue(app_);
  apps::AppLoadService::Get(profile_)->RestartApplicationIfRunning(app_->id());

  Close();
}

int AppInfoPermissionsPanel::GetRetainedDeviceCount() const {
  return extensions::DevicePermissionsManager::Get(profile_)
      ->GetPermissionMessageStrings(app_->id())
      .size();
}

base::string16 AppInfoPermissionsPanel::GetRetainedDeviceHeading() const {
  const int kRetainedDevicesMessageIDs[6] = {
      IDS_APPLICATION_INFO_RETAINED_DEVICES_DEFAULT,
      IDS_APPLICATION_INFO_RETAINED_DEVICE_SINGULAR,
      IDS_APPLICATION_INFO_RETAINED_DEVICES_ZERO,
      IDS_APPLICATION_INFO_RETAINED_DEVICES_TWO,
      IDS_APPLICATION_INFO_RETAINED_DEVICES_FEW,
      IDS_APPLICATION_INFO_RETAINED_DEVICES_MANY,
  };
  std::vector<int> message_ids(
      kRetainedDevicesMessageIDs,
      kRetainedDevicesMessageIDs + arraysize(kRetainedDevicesMessageIDs));

  return l10n_util::GetPluralStringFUTF16(message_ids,
                                          GetRetainedDeviceCount());
}

const std::vector<base::string16> AppInfoPermissionsPanel::GetRetainedDevices()
    const {
  return extensions::DevicePermissionsManager::Get(profile_)
      ->GetPermissionMessageStrings(app_->id());
}

void AppInfoPermissionsPanel::RevokeDevicePermissions() {
  extensions::DevicePermissionsManager::Get(profile_)->Clear(app_->id());
  apps::AppLoadService::Get(profile_)->RestartApplicationIfRunning(app_->id());

  Close();
}
