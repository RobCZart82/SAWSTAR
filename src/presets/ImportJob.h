// SPDX-License-Identifier: MIT
#pragma once
#include "presets/Library.h"
#include <condition_variable>
#include <functional>
#include <mutex>
#include <optional>
#include <thread>

namespace sawstar {
struct PresetImportRequest {
  std::vector<fs::path> files;
  fs::path root;
  bool allowIdentical = false;
};
struct PresetImportResult {
  ImportReport report;
  std::unique_ptr<PresetLibrary> library;
  std::string error;
  bool allowIdentical = false;
};
inline PresetImportResult ExecutePresetImport(const PresetImportRequest& request) {
  PresetImportResult result;
  result.allowIdentical = request.allowIdentical;
  try {
    result.report = ImportPresets(request.files, request.root, request.allowIdentical);
    // Directory enumeration and favorites reads also belong to the worker.
    result.library = std::make_unique<PresetLibrary>(request.root);
  } catch (const std::exception& error) { result.error = error.what(); }
  return result;
}

// Owned by the plugin instance, not by an editor/control. No GUI, plugin or
// parameter pointers are carried by a request/result. Never call on audio thread.
class PresetImportJob {
public:
  using Work = std::function<PresetImportResult(const PresetImportRequest&)>;
  explicit PresetImportJob(Work work = ExecutePresetImport):work_(std::move(work)) {}
  PresetImportJob(const PresetImportJob&) = delete;
  PresetImportJob& operator=(const PresetImportJob&) = delete;
  ~PresetImportJob() {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      stopping_ = true;
    }
    wake_.notify_one();
    // Editor close does not destroy this job. Plugin unload joins any active
    // batch before its code/data can be unloaded. A queued batch may be cancelled.
    if (worker_.joinable()) worker_.join();
  }
  bool Start(PresetImportRequest request) {
    std::lock_guard<std::mutex> lock(mutex_);
    // Completed-but-unread results stay busy, including while the editor is shut.
    if (busy_ || stopping_) return false;
    request_ = std::move(request);
    busy_ = true;
    try {
      if (!worker_.joinable()) worker_ = std::thread([this]{Run();});
    } catch (...) {
      request_.reset();busy_ = false;throw;
    }
    wake_.notify_one();
    return true;
  }
  bool Busy() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return busy_;
  }
  std::optional<PresetImportResult> TakeResult() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!result_) return {};
    auto result = std::move(result_);
    result_.reset();busy_ = false;
    return result;
  }
private:
  void Run() {
    for (;;) {
      PresetImportRequest request;
      {
        std::unique_lock<std::mutex> lock(mutex_);
        wake_.wait(lock,[this]{return stopping_ || request_.has_value();});
        if (stopping_) return;
        request = std::move(*request_);request_.reset();
      }
      PresetImportResult result;
      result.allowIdentical = request.allowIdentical;
      try { result = work_(request); }
      catch (const std::exception& error) { result.error = error.what(); }
      catch (...) { result.error = "Preset import failed."; }
      {
        std::lock_guard<std::mutex> lock(mutex_);
        result_ = std::move(result);
      }
    }
  }
  Work work_;
  mutable std::mutex mutex_;
  std::condition_variable wake_;
  std::optional<PresetImportRequest> request_;
  std::optional<PresetImportResult> result_;
  bool busy_ = false, stopping_ = false;
  std::thread worker_;
};
}
