// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef EXTENSIONS_BROWSER_API_SYSTEM_DISPLAY_SYSTEM_DISPLAY_API_H_
#define EXTENSIONS_BROWSER_API_SYSTEM_DISPLAY_SYSTEM_DISPLAY_API_H_

#include <string>

#include "extensions/browser/api/system_display/display_info_provider.h"
#include "extensions/browser/extension_function.h"

namespace extensions {

class SystemDisplayFunction : public UIThreadExtensionFunction {
 public:
  static const char kCrosOnlyError[];
  static const char kKioskOnlyError[];

 protected:
  ~SystemDisplayFunction() override {}
  bool PreRunValidation(std::string* error) override;

  // Returns true if this function should be restricted to kiosk-mode apps and
  // webui. The default is true.
  virtual bool ShouldRestrictToKioskAndWebUI();
};

// GetInfo is a SystemDisplayFunction to limit some of the fields to kiosk mode
// and web UI. The method itself is not kiosk-mode only.
class SystemDisplayGetInfoFunction : public SystemDisplayFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("system.display.getInfo", SYSTEM_DISPLAY_GETINFO);

 protected:
  ~SystemDisplayGetInfoFunction() override {}

  bool PreRunValidation(std::string* error) override;
  ResponseAction Run() override;

  void Response(DisplayInfoProvider::DisplayUnitInfoList all_displays_info);
};

class SystemDisplayGetDisplayLayoutFunction : public SystemDisplayFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("system.display.getDisplayLayout",
                             SYSTEM_DISPLAY_GETDISPLAYLAYOUT);

 protected:
  ~SystemDisplayGetDisplayLayoutFunction() override {}
  ResponseAction Run() override;
  bool ShouldRestrictToKioskAndWebUI() override;

  void Response(DisplayInfoProvider::DisplayLayoutList display_layout);
};

class SystemDisplaySetDisplayPropertiesFunction : public SystemDisplayFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("system.display.setDisplayProperties",
                             SYSTEM_DISPLAY_SETDISPLAYPROPERTIES);

 protected:
  ~SystemDisplaySetDisplayPropertiesFunction() override {}
  ResponseAction Run() override;

  void Response(base::Optional<std::string> error);
};

class SystemDisplaySetDisplayLayoutFunction : public SystemDisplayFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("system.display.setDisplayLayout",
                             SYSTEM_DISPLAY_SETDISPLAYLAYOUT);

 protected:
  ~SystemDisplaySetDisplayLayoutFunction() override {}
  ResponseAction Run() override;

  void Response(base::Optional<std::string> error);
};

class SystemDisplayEnableUnifiedDesktopFunction : public SystemDisplayFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("system.display.enableUnifiedDesktop",
                             SYSTEM_DISPLAY_ENABLEUNIFIEDDESKTOP);

 protected:
  ~SystemDisplayEnableUnifiedDesktopFunction() override {}
  ResponseAction Run() override;
};

class SystemDisplayOverscanCalibrationStartFunction
    : public SystemDisplayFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("system.display.overscanCalibrationStart",
                             SYSTEM_DISPLAY_OVERSCANCALIBRATIONSTART);

 protected:
  ~SystemDisplayOverscanCalibrationStartFunction() override {}
  ResponseAction Run() override;
};

class SystemDisplayOverscanCalibrationAdjustFunction
    : public SystemDisplayFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("system.display.overscanCalibrationAdjust",
                             SYSTEM_DISPLAY_OVERSCANCALIBRATIONADJUST);

 protected:
  ~SystemDisplayOverscanCalibrationAdjustFunction() override {}
  ResponseAction Run() override;
};

class SystemDisplayOverscanCalibrationResetFunction
    : public SystemDisplayFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("system.display.overscanCalibrationReset",
                             SYSTEM_DISPLAY_OVERSCANCALIBRATIONRESET);

 protected:
  ~SystemDisplayOverscanCalibrationResetFunction() override {}
  ResponseAction Run() override;
};

class SystemDisplayOverscanCalibrationCompleteFunction
    : public SystemDisplayFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("system.display.overscanCalibrationComplete",
                             SYSTEM_DISPLAY_OVERSCANCALIBRATIONCOMPLETE);

 protected:
  ~SystemDisplayOverscanCalibrationCompleteFunction() override {}
  ResponseAction Run() override;
};

class SystemDisplayShowNativeTouchCalibrationFunction
    : public SystemDisplayFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("system.display.showNativeTouchCalibration",
                             SYSTEM_DISPLAY_SHOWNATIVETOUCHCALIBRATION);

 protected:
  ~SystemDisplayShowNativeTouchCalibrationFunction() override {}
  ResponseAction Run() override;

  void OnCalibrationComplete(base::Optional<std::string> error);
};

class SystemDisplayStartCustomTouchCalibrationFunction
    : public SystemDisplayFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("system.display.startCustomTouchCalibration",
                             SYSTEM_DISPLAY_STARTCUSTOMTOUCHCALIBRATION);

 protected:
  ~SystemDisplayStartCustomTouchCalibrationFunction() override {}
  ResponseAction Run() override;
};

class SystemDisplayCompleteCustomTouchCalibrationFunction
    : public SystemDisplayFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("system.display.completeCustomTouchCalibration",
                             SYSTEM_DISPLAY_COMPLETECUSTOMTOUCHCALIBRATION);

 protected:
  ~SystemDisplayCompleteCustomTouchCalibrationFunction() override {}
  ResponseAction Run() override;
};

class SystemDisplayClearTouchCalibrationFunction
    : public SystemDisplayFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("system.display.clearTouchCalibration",
                             SYSTEM_DISPLAY_CLEARTOUCHCALIBRATION);

 protected:
  ~SystemDisplayClearTouchCalibrationFunction() override {}
  ResponseAction Run() override;
};

class SystemDisplaySetMirrorModeFunction : public SystemDisplayFunction {
 public:
  DECLARE_EXTENSION_FUNCTION("system.display.setMirrorMode",
                             SYSTEM_DISPLAY_SETMIRRORMODE);

 protected:
  ~SystemDisplaySetMirrorModeFunction() override {}
  ResponseAction Run() override;

  void Response(base::Optional<std::string> error);
};

}  // namespace extensions

#endif  // EXTENSIONS_BROWSER_API_SYSTEM_DISPLAY_SYSTEM_DISPLAY_API_H_
