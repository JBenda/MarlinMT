#include "marlin/book/StoreWriter.h"

// -- std includes
#include <filesystem>
#include <unordered_map>
#include <vector>

// -- MarlinBook includes
#include "marlin/book/configs/ROOTv7.h"
#include "marlin/book/Entry.h"
#include "marlin/book/Handle.h"
#include "marlin/book/Hist.h"
#include "marlin/book/Selection.h"
#include "marlin/book/Types.h"


// -- ROOT includes
#include "TDirectory.h"
#include "TDirectoryFile.h"
#include "TFile.h"

class PathHash {
public:
  std::size_t operator()(const std::filesystem::path& path) const {
    return std::filesystem::hash_value(path);
  }
};
using DirectoryMap = std::unordered_map<std::filesystem::path, TDirectory*, PathHash>;

template<typename T>  
void writeObject( TDirectory* file, const std::string_view& name, const T& obj) {

  auto root6Obj = marlin::book::types::toRoot6(obj, name);

  if constexpr (!std::is_same_v<decltype(toRoot6(obj,name)), decltype(nullptr)>) {
    file->WriteTObject(
        &root6Obj,
        std::string(name).c_str()
    ) ;
  }
}



namespace marlin {
  namespace book {

    void StoreWriter::writeSelection(
      const Selection             &selection
    ) {
      DirectoryMap dirs{};

      TFile root(_path.string().c_str(), "UPDATE");
      if(root.IsZombie()) {
        MARLIN_BOOK_THROW(std::string("failed to create file: ") + _path.string());
      }
      for(const WeakEntry& h : selection) {
        if(!h.valid()) { continue; }
        const EntryKey& key = h.key();
        const std::type_index type = key.type;
        std::string path = std::filesystem::relative(key.path, "/").remove_filename().string();
        path.pop_back();

        const char* c_path = path.c_str();
        TDirectory *file = root.GetDirectory(c_path);
        if(file == nullptr) {
          root.mkdir(c_path);
          file = root.GetDirectory(c_path);
        }

        if(file == nullptr) {
          MARLIN_BOOK_THROW( std::string("failed create: ") + path + '\n');
        }
        if (type == std::type_index(typeid(types::H1F))) {
          writeObject<types::H1F>(file, key.path.filename().string(), h.handle<types::H1F>().merged());
        } else if (type == std::type_index(typeid(types::H1D))){ 
          writeObject<types::H1D>(file, key.path.filename().string(), h.handle<types::H1D>().merged());
        } else if (type == std::type_index(typeid(types::H1I))){
          writeObject<types::H1I>(file, key.path.filename().string(), h.handle<types::H1I>().merged());
        } else if (type == std::type_index(typeid(types::H2F))){ 
          writeObject<types::H2F>(file, key.path.filename().string(), h.handle<types::H2F>().merged());
        } else if (type == std::type_index(typeid(types::H2D))){
          writeObject<types::H2D>(file, key.path.filename().string(), h.handle<types::H2D>().merged());
        } else if (type == std::type_index(typeid(types::H2I))){ 
          writeObject<types::H2I>(file, key.path.filename().string(), h.handle<types::H2I>().merged());
        } else if (type == std::type_index(typeid(types::H3F))){
          writeObject<types::H3F>(file, key.path.filename().string(), h.handle<types::H3F>().merged());
        } else if (type == std::type_index(typeid(types::H3D))){ 
          writeObject<types::H3D>(file, key.path.filename().string(), h.handle<types::H3D>().merged());
        } else if (type == std::type_index(typeid(types::H3I))){
          writeObject<types::H3I>(file, key.path.filename().string(), h.handle<types::H3I>().merged());
        } else {
          MARLIN_BOOK_THROW( "can't store object, no known operation" );
        }
      }
      root.Close();
        
    }

  } // end namespace book
} // end namespace marlin

