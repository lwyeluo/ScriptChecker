// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef IOS_CHROME_BROWSER_UI_SETTINGS_PASSWORD_EXPORTER_H_
#define IOS_CHROME_BROWSER_UI_SETTINGS_PASSWORD_EXPORTER_H_

#import <Foundation/Foundation.h>

#include <memory>
#include <vector>

namespace autofill {
struct PasswordForm;
}  // namespace autofill

enum class WriteToURLStatus {
  SUCCESS,
  OUT_OF_DISK_SPACE_ERROR,
  UNKNOWN_ERROR,
};

enum class ExportState {
  IDLE,
  ONGOING,
  CANCELLING,
};

@protocol ReauthenticationProtocol;

@protocol FileWriterProtocol<NSObject>

// Posts a task to write the data in |data| to the file at |fileURL| and
// executes |handler| when the writing is finished.
- (void)writeData:(NSString*)data
            toURL:(NSURL*)fileURL
          handler:(void (^)(WriteToURLStatus))handler;

@end

@protocol PasswordSerializerBridge<NSObject>

// Posts task to serialize passwords and calls |serializedPasswordsHandler|
// when serialization is finished.
- (void)serializePasswords:
            (std::vector<std::unique_ptr<autofill::PasswordForm>>)passwords
                   handler:(void (^)(std::string))serializedPasswordsHandler;

@end

@protocol PasswordExporterDelegate<NSObject>

// Displays a dialog informing the user that they must set up a passcode
// in order to export passwords.
- (void)showSetPasscodeDialog;

// Displays an alert which informs the user that the passwords are being
// prepared to be exported and gives them the option of cancelling the export.
- (void)showPreparingPasswordsAlert;

// Displays an alert detailing an error that has occured during export.
- (void)showExportErrorAlertWithLocalizedReason:(NSString*)errorReason;

// Displays an activity view that allows the user to pick an app to process
// the exported passwords file.
- (void)showActivityViewWithActivityItems:(NSArray*)activityItems
                        completionHandler:
                            (void (^)(NSString* activityType,
                                      BOOL completed,
                                      NSArray* returnedItems,
                                      NSError* activityError))completionHandler;

// Enables or disables the export button based on the export state.
- (void)updateExportPasswordsButton;

@end

/** Class handling all the operations necessary to export passwords.*/
@interface PasswordExporter : NSObject

// The designated initializer. |reauthenticationModule| and |delegate| must
// not be nil.
- (instancetype)initWithReauthenticationModule:
                    (id<ReauthenticationProtocol>)reuthenticationModule
                                      delegate:
                                          (id<PasswordExporterDelegate>)delegate
    NS_DESIGNATED_INITIALIZER;

- (instancetype)init NS_UNAVAILABLE;

// Method to be called in order to start the export flow. This initiates
// the reauthentication procedure and asks for password serialization.
- (void)startExportFlow:
    (std::vector<std::unique_ptr<autofill::PasswordForm>>)passwords;

// Called when the user cancels the export operation.
- (void)cancelExport;

// Called to re-enable export functionality when the export UI flow finishes.
- (void)resetExportState;

// State of the export operation.
@property(nonatomic, readonly, assign) ExportState exportState;

@end

#endif  // IOS_CHROME_BROWSER_UI_SETTINGS_PASSWORD_EXPORTER_H_
