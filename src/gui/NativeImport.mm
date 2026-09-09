// SPDX-License-Identifier: MIT
#import <Cocoa/Cocoa.h>
#include "gui/NativeImport.h"
namespace sawstar::gui {
std::vector<std::filesystem::path> SelectPresetFiles(void*){
 @autoreleasepool {
  NSOpenPanel* panel=[NSOpenPanel openPanel];[panel setTitle:@"Import SAWSTAR presets"];
  [panel setAllowsMultipleSelection:YES];[panel setCanChooseDirectories:NO];[panel setCanChooseFiles:YES];[panel setAllowedFileTypes:@[@"sawstar"]];
  std::vector<std::filesystem::path> files;
  if([panel runModal]==NSModalResponseOK)for(NSURL* url in [panel URLs])files.push_back(std::filesystem::u8path([[url path] UTF8String]));
  return files;
 }
}
}
