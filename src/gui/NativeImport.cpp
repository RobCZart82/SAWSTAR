// SPDX-License-Identifier: MIT
#include "gui/NativeImport.h"
#include <windows.h>
#include <commdlg.h>
#include <stdexcept>
namespace sawstar::gui {
std::vector<std::filesystem::path> SelectPresetFiles(void* parent){
 std::vector<wchar_t> buffer(65536);OPENFILENAMEW dialog{};dialog.lStructSize=sizeof(dialog);dialog.hwndOwner=static_cast<HWND>(parent);
 dialog.lpstrFilter=L"SAWSTAR presets\0*.sawstar\0\0";dialog.lpstrFile=buffer.data();dialog.nMaxFile=static_cast<DWORD>(buffer.size());dialog.lpstrTitle=L"Import SAWSTAR presets";
 dialog.Flags=OFN_EXPLORER|OFN_ALLOWMULTISELECT|OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR;
 if(!GetOpenFileNameW(&dialog)){if(CommDlgExtendedError())throw std::runtime_error("File selection failed. Try fewer files at once.");return {};}
 std::filesystem::path first(buffer.data());const wchar_t* p=buffer.data()+wcslen(buffer.data())+1;
 if(!*p)return {first};std::vector<std::filesystem::path> files;while(*p){files.push_back(first/p);p+=wcslen(p)+1;}return files;
}
}
