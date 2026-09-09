// SPDX-License-Identifier: MIT
#pragma once
#include "presets/UserPresets.h"
#include "presets/FactoryPresets.h"
#include <set>
#include <iomanip>
#include <cctype>
namespace sawstar {
inline std::string Fold(std::string s){for(auto& c:s)if(static_cast<unsigned char>(c)<128)c=char(std::tolower(static_cast<unsigned char>(c)));return s;}
struct ImportReport {
 int imported=0,skipped=0,failed=0;std::vector<std::string> details;
 std::string Summary()const{return std::to_string(imported)+" imported, "+std::to_string(skipped)+" skipped, "+std::to_string(failed)+" failed.";}
};
inline ImportReport ImportPresets(const std::vector<fs::path>& files,const fs::path& root){
 if(root.empty())throw std::runtime_error("User preset folder is unavailable.");fs::create_directories(root);ImportReport report;
 for(const auto& p:files)try{
  if(Fold(p.extension().u8string())!=".sawstar"||!ValidPresetName(p.stem().u8string()))throw std::runtime_error("Invalid preset filename.");
  ReadUserPreset(p);auto target=root/fs::u8path(p.stem().u8string()+".sawstar");
  // Case-insensitive collision policy is consistent on macOS and Windows.
  bool collision=false;for(const auto& entry:ListUserPresets(root))if(Fold(entry.filename().u8string())==Fold(target.filename().u8string())){collision=true;break;}
  if(collision){++report.skipped;report.details.push_back(p.filename().u8string()+": already in library; not overwritten.");continue;}
  if(!fs::copy_file(p,target,fs::copy_options::none))throw std::runtime_error("Could not copy preset.");++report.imported;
 }catch(const std::exception& e){++report.failed;report.details.push_back(p.filename().u8string()+": "+e.what());}
 return report;
}
struct LibraryEntry {
 std::string key,name,category,lesson;fs::path path;int factory=-1;
 Snapshot Read()const{return factory>=0?FactoryPresets()[factory].values:ReadUserPreset(path);}
};
class PresetLibrary {
 fs::path root_;std::set<std::string> favorites_;
public:
 std::vector<LibraryEntry> entries;std::string warning;
 explicit PresetLibrary(fs::path root):root_(std::move(root)){
  if(!root_.empty()){std::ifstream in(root_/"favorites.txt");std::string key;while(in>>std::quoted(key))favorites_.insert(key);}Refresh();
 }
 void Refresh(){ReloadFavorites();entries.clear();int i=0;for(const auto& p:FactoryPresets()){entries.push_back({"factory:"+std::string(p.key),p.name,p.category,p.lesson,{},i++});}
  warning.clear();if(root_.empty())return;try{for(const auto& p:ListUserPresets(root_))entries.push_back({"user:"+p.filename().u8string(),p.stem().u8string(),"User","User sound. The diagram below describes its saved settings.",p,-1});}catch(const std::exception& e){warning=e.what();}}
 bool Favorite(const std::string& key)const{return favorites_.count(key)>0;}
 void ToggleFavorite(const std::string& key){ReloadFavorites();auto next=favorites_;if(next.count(key))next.erase(key);else next.insert(key);SaveFavorites(next);favorites_=std::move(next);}
 void MoveFavorite(const std::string& oldKey,const std::string& newKey){ReloadFavorites();if(!Favorite(oldKey))return;auto next=favorites_;next.erase(oldKey);next.insert(newKey);SaveFavorites(next);favorites_=std::move(next);}
 std::vector<int> Filter(const std::string& category,const std::string& query)const{
  std::vector<int> out;auto q=Fold(query);for(int i=0;i<int(entries.size());++i){const auto& e=entries[i];bool match=category=="All"||(category=="Favorites"?Favorite(e.key):category=="Init"?e.factory==0:category=="User"?e.factory<0:category==e.category);
   if(match&&Fold(e.name+" "+e.category).find(q)!=std::string::npos)out.push_back(i);}return out;
 }
private:
 void ReloadFavorites(){if(root_.empty())return;std::ifstream in(root_/"favorites.txt");if(!in)return;std::set<std::string> values;std::string key;while(in>>std::quoted(key))values.insert(key);favorites_=std::move(values);}
 void SaveFavorites(const std::set<std::string>& values){if(root_.empty())throw std::runtime_error("User folder unavailable.");fs::create_directories(root_);std::ofstream out(root_/"favorites.txt",std::ios::trunc);if(!out)throw std::runtime_error("Cannot save favorites.");for(const auto& k:values)out<<std::quoted(k)<<'\n';out.close();if(!out)throw std::runtime_error("Cannot write favorites.");}
};
}
