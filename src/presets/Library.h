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
// Serialized with Save, Rename, Overwrite and Archive, including collision checks.
ImportReport ImportPresets(const std::vector<fs::path>& files,const fs::path& root,bool allowIdentical=false);
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
