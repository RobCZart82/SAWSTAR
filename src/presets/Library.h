// SPDX-License-Identifier: MIT
#pragma once
#include "presets/UserPresets.h"
#include "presets/FactoryPresets.h"
#include <set>
#include <iomanip>
#include <cctype>
namespace sawstar {
// UI/file-management thread only. The implementation serializes across instances
// and processes, and replaces the complete favorites file atomically.
std::set<std::string> ReadFavorites(const fs::path& root);
std::set<std::string> ChangeFavorite(const fs::path& root,const std::string& key,const std::string* moveTo=nullptr);
struct ImportReport {
 std::vector<fs::path> duplicates;
 int imported=0,skipped=0,failed=0;std::vector<std::string> details;
 std::string Summary()const{return std::to_string(imported)+" imported, "+std::to_string(skipped)+" skipped, "+std::to_string(failed)+" failed.";}
};
inline ImportReport ImportPresets(const std::vector<fs::path>& files,const fs::path& root,bool allowIdentical=false){
 if(root.empty())throw std::runtime_error("User preset folder is unavailable.");fs::create_directories(root);ImportReport report;
 for(const auto& p:files)try{
  if(Fold(p.extension().u8string())!=".sawstar"||!ValidPresetName(p.stem().u8string()))throw std::runtime_error("Invalid preset filename.");
  auto values=ReadUserPreset(p);auto target=root/fs::u8path(p.stem().u8string()+".sawstar");
  // Case-insensitive collision policy is consistent on macOS and Windows.
  bool collision=false;for(const auto& entry:ListUserPresets(root))if(Fold(entry.filename().u8string())==Fold(target.filename().u8string())){collision=true;break;}
  if(collision){++report.skipped;report.details.push_back(p.filename().u8string()+": already in library; not overwritten.");continue;}
  if(!allowIdentical){std::string match;for(const auto& entry:ListUserPresets(root)){try{if(ReadUserPreset(entry)==values){match=entry.stem().u8string();break;}}catch(const std::exception&){/* A damaged existing file must not block valid imports. */}}if(!match.empty()){++report.skipped;report.duplicates.push_back(p);report.details.push_back(p.filename().u8string()+": same settings as "+match);continue;}}
  if(!fs::copy_file(p,target,fs::copy_options::none))throw std::runtime_error("Could not copy preset.");++report.imported;
 }catch(const std::exception& e){++report.failed;report.details.push_back(p.filename().u8string()+": "+e.what());}
 return report;
}
// A successful external save remains active even if its managed copy fails.
inline UserPresetSelection SavePresetSelection(fs::path path,const Snapshot& values,const fs::path& root){
 if(path.extension().empty())path+=".sawstar";
 if(!PresetExtension(path)||!ValidPresetName(path.stem().u8string()))throw std::runtime_error("Use a valid preset name and .sawstar extension.");
 SaveUserPreset(path,values);
 UserPresetSelection result;result.path=path;result.name=path.stem().u8string();result.saved=values;result.active=true;result.status="Saved "+result.name;
 try{if(path.parent_path()!=root){auto report=ImportPresets({path},root,true);if(report.imported)result.path=root/fs::u8path(path.stem().u8string()+".sawstar");else result.status="Saved externally; library copy skipped. "+report.Summary();}}
 catch(const std::exception&){result.status="Saved externally; library copy could not be created.";}
 return result;
}
struct LibraryEntry {
 std::string key,name,category,lesson;fs::path path;int factory=-1;std::string tags{};
 Snapshot Read()const{return factory>=0?FactoryPresets()[factory].values:ReadUserPreset(path);}
};
class PresetLibrary {
 fs::path root_;std::set<std::string> favorites_;
public:
 std::vector<LibraryEntry> entries;std::string warning;
 explicit PresetLibrary(fs::path root):root_(std::move(root)){Refresh();}
 void Refresh(){warning.clear();try{ReloadFavorites();}catch(const std::exception& e){warning=e.what();}entries.clear();int i=0;for(const auto& p:FactoryPresets()){entries.push_back({"factory:"+std::string(p.key),p.name,p.category,p.lesson,{},i++,p.tags});}
  if(root_.empty())return;try{for(const auto& p:ListUserPresets(root_))entries.push_back({"user:"+p.filename().u8string(),p.stem().u8string(),"User","User sound. The diagram below describes its saved settings.",p,-1});}catch(const std::exception& e){warning=e.what();}}
 bool Favorite(const std::string& key)const{return favorites_.count(key)>0;}
 void ToggleFavorite(const std::string& key){favorites_=ChangeFavorite(root_,key);}
 void MoveFavorite(const std::string& oldKey,const std::string& newKey){favorites_=ChangeFavorite(root_,oldKey,&newKey);}
 std::vector<int> Filter(const std::string& category,const std::string& query)const{
  std::vector<int> out;auto q=Fold(query);for(int i=0;i<int(entries.size());++i){const auto& e=entries[i];bool match=category=="All"||(category=="Favorites"?Favorite(e.key):category=="User"?e.factory<0:category==e.category);
   if(match&&Fold(e.name+" "+e.category+" "+e.tags).find(q)!=std::string::npos)out.push_back(i);}return out;
 }
private:
 void ReloadFavorites(){favorites_=ReadFavorites(root_);}
};
}
