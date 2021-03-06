// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/system/rotation/rotation_lock_feature_pod_controller.h"

#include "ash/shell.h"
#include "ash/system/unified/feature_pod_button.h"
#include "ash/test/ash_test_base.h"
#include "ash/wm/tablet_mode/tablet_mode_controller.h"
#include "base/command_line.h"
#include "ui/display/display_switches.h"

namespace ash {

class RotationLockFeaturePodControllerTest : public AshTestBase {
 public:
  RotationLockFeaturePodControllerTest() = default;
  ~RotationLockFeaturePodControllerTest() override = default;

  // AshTestBase:
  void SetUp() override;
  void TearDown() override;

 protected:
  void SetUpController();

  RotationLockFeaturePodController* controller() const {
    return controller_.get();
  }

  FeaturePodButton* button_view() const { return button_view_.get(); }

 private:
  std::unique_ptr<RotationLockFeaturePodController> controller_;
  std::unique_ptr<FeaturePodButton> button_view_;

  DISALLOW_COPY_AND_ASSIGN(RotationLockFeaturePodControllerTest);
};

void RotationLockFeaturePodControllerTest::SetUp() {
  // The Display used for testing is not an internal display. This flag
  // allows for DisplayManager to treat it as one.
  base::CommandLine::ForCurrentProcess()->AppendSwitch(
      ::switches::kUseFirstDisplayAsInternal);
  AshTestBase::SetUp();
}

void RotationLockFeaturePodControllerTest::TearDown() {
  controller_.reset();
  button_view_.reset();
  AshTestBase::TearDown();
}

void RotationLockFeaturePodControllerTest::SetUpController() {
  controller_ = std::make_unique<RotationLockFeaturePodController>();
  button_view_.reset(controller_->CreateButton());
}

// Tests that when the button is initially created, that it is created
// not visible.
TEST_F(RotationLockFeaturePodControllerTest, CreateButton) {
  SetUpController();
  EXPECT_FALSE(button_view()->visible());
}

// Tests that when the button is created, while TabletMode is active,
// that it is visible.
TEST_F(RotationLockFeaturePodControllerTest, CreateButtonDuringTabletMode) {
  Shell::Get()->tablet_mode_controller()->EnableTabletModeWindowManager(true);
  SetUpController();
  EXPECT_TRUE(button_view()->visible());
  Shell::Get()->tablet_mode_controller()->EnableTabletModeWindowManager(false);
  EXPECT_FALSE(button_view()->visible());
}

// Tests that the enabling of TabletMode affects a previously created default
// view, changing the visibility.
TEST_F(RotationLockFeaturePodControllerTest,
       ButtonVisibilityChangesDuringTabletMode) {
  SetUpController();
  Shell::Get()->tablet_mode_controller()->EnableTabletModeWindowManager(true);
  EXPECT_TRUE(button_view()->visible());
  Shell::Get()->tablet_mode_controller()->EnableTabletModeWindowManager(false);
  EXPECT_FALSE(button_view()->visible());
}

TEST_F(RotationLockFeaturePodControllerTest, OnPressed) {
  SetUpController();
  TabletModeController* tablet_mode_controller =
      Shell::Get()->tablet_mode_controller();
  ScreenOrientationController* screen_orientation_controller =
      Shell::Get()->screen_orientation_controller();
  ASSERT_FALSE(screen_orientation_controller->rotation_locked());
  tablet_mode_controller->EnableTabletModeWindowManager(true);
  ASSERT_TRUE(button_view()->visible());
  EXPECT_FALSE(button_view()->IsToggled());

  controller()->OnPressed();
  EXPECT_TRUE(screen_orientation_controller->rotation_locked());
  EXPECT_TRUE(button_view()->visible());
  EXPECT_TRUE(button_view()->IsToggled());

  controller()->OnPressed();
  EXPECT_FALSE(screen_orientation_controller->rotation_locked());
  EXPECT_TRUE(button_view()->visible());
  EXPECT_FALSE(button_view()->IsToggled());

  tablet_mode_controller->EnableTabletModeWindowManager(false);
}

}  // namespace ash
