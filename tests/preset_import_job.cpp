// SPDX-License-Identifier: MIT
#include "presets/ImportJob.h"
#include <chrono>
#include <future>
#include <atomic>
#include <iostream>
using namespace sawstar;
namespace {
void Check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
struct Gate {
 std::mutex mutex;std::condition_variable wake;bool entered=false,open=false;
 void Wait(){std::unique_lock<std::mutex> lock(mutex);entered=true;wake.notify_all();Check(wake.wait_for(lock,std::chrono::seconds(10),[&]{return open;}),"test gate timed out");}
 void Entered(){std::unique_lock<std::mutex> lock(mutex);Check(wake.wait_for(lock,std::chrono::seconds(5),[&]{return entered;}),"worker did not start");}
 void Open(){std::lock_guard<std::mutex> lock(mutex);open=true;wake.notify_all();}
 ~Gate(){Open();}
};
std::optional<PresetImportResult> Take(PresetImportJob& job){
 auto end=std::chrono::steady_clock::now()+std::chrono::seconds(10);
 while(std::chrono::steady_clock::now()<end){if(auto result=job.TakeResult())return result;std::this_thread::yield();}
 throw std::runtime_error("import job did not complete");
}
}
int main(){auto root=fs::temp_directory_path()/("sawstar-import-job-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));try{
 // A blocked worker must not hold the job mutex or make Start/poll wait for I/O.
 Gate gate;const auto owner=std::this_thread::get_id();PresetImportJob job([&](const PresetImportRequest& request){Check(std::this_thread::get_id()!=owner,"import executed on owner thread");gate.Wait();return ExecutePresetImport(request);});
 PresetLibrary deferred(root/"unopened",false);Check(deferred.entries.empty()&&!fs::exists(root/"unopened"),"deferred browser construction performed refresh");
 auto sound=DefaultSnapshot();auto source=root/"source"/"Sound.sawstar";SaveUserPreset(source,sound);
 Check(job.Start({{source},root/"library",false}),"job did not start");gate.Entered();
 Check(job.Busy()&&!job.Start({{},root/"other",false})&&!job.TakeResult(),"busy job accepted replacement or blocked polling");
 // Closing a hypothetical editor releases no worker-owned data. A later editor
 // can retrieve the result from the same plugin-owned job.
 gate.Open();auto result=Take(job);
 Check(result->error.empty()&&result->report.imported==1&&result->library&&result->library->Filter("User","").size()==1&&!job.Busy(),"worker import or library snapshot failed");
 Check(ReadUserPreset(root/"library"/"Sound.sawstar")==sound,"worker changed sound");
 Check(!job.TakeResult(),"completion delivered twice");
 auto invalid=root/"source"/"Invalid.sawstar";{std::ofstream file(invalid);file<<"invalid";}
 Check(job.Start({{source,invalid,root/"source"/"Missing.sawstar"},root/"library",false}),"mixed batch rejected");result=Take(job);
 Check(result->report.imported==0&&result->report.skipped==1&&result->report.failed==2&&ReadUserPreset(root/"library"/"Sound.sawstar")==sound,"mixed batch reporting or collision preservation failed");
 auto copy=root/"source"/"Copy.sawstar";SaveUserPreset(copy,sound);
 Check(job.Start({{copy},root/"library",false}),"second job rejected");result=Take(job);
 Check(result->report.skipped==1&&result->report.duplicates.size()==1&&!result->allowIdentical,"duplicate report lost");
 Check(job.Start({result->report.duplicates,root/"library",true}),"duplicate retry rejected");result=Take(job);
 Check(result->allowIdentical&&result->report.imported==1&&ReadUserPreset(root/"library"/"Copy.sawstar")==sound,"confirmed duplicate retry failed");
 // An unread completion cannot be replaced while no editor is consuming it.
 std::promise<void> executed;auto completed=executed.get_future();
 PresetImportJob retained([&](const PresetImportRequest& request){auto value=ExecutePresetImport(request);executed.set_value();return value;});
 Check(retained.Start({{},root/"library",false}),"retained job rejected");Check(completed.wait_for(std::chrono::seconds(10))==std::future_status::ready,"retained job did not execute");
 Check(retained.Busy()&&!retained.Start({{},root/"other",false}),"unread result was replaceable");
 result=Take(retained);Check(result->error.empty()&&!retained.Busy(),"retained completion lost");
 PresetImportJob errors;Check(errors.Start({{}, {},false}),"error job rejected");result=Take(errors);
 Check(!result->error.empty()&&!errors.Busy(),"root error not reported");
 Check(errors.Start({{},root/"library",false}),"job unusable after failure");result=Take(errors);Check(result->error.empty(),"recovery job failed");
 // Plugin destruction joins a running worker. Release the gate from another
 // thread; joining must finish before executor storage or plugin code disappears.
 Gate shutdown;std::atomic<bool> finished{false};
 auto owned=std::make_unique<PresetImportJob>([&](const PresetImportRequest&){shutdown.Wait();finished=true;return PresetImportResult{};});
 Check(owned->Start({{},root/"library",false}),"shutdown job rejected");shutdown.Entered();
 std::thread release([&]{shutdown.Open();});owned.reset();release.join();Check(finished,"destructor left worker running");
 fs::remove_all(root);std::cout<<"Background import, retained completion, duplicate retry, error recovery and shutdown passed\n";
}catch(const std::exception& error){std::error_code ignored;fs::remove_all(root,ignored);std::cerr<<error.what()<<'\n';return 1;}}
