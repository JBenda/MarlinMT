
// -- marlin headers
#include <marlin/BookStoreManager.h>
#include <marlin/Application.h>
#include <marlin/Exceptions.h>
#include <marlin/Processor.h>

namespace marlin {

  void BookStoreManager::init( const Application *app ) {
    if( isInitialized() ) {
      MARLIN_THROW( "BookStoreManager::init: Already initialized !" ) ;
    }
    _application = app ;
    _logger = _application->createLogger( "BookStoreManager" ) ;
  }
  
  //--------------------------------------------------------------------------
  
  bool BookStoreManager::isInitialized() const {
    return ( nullptr != _application ) ;
  }

  //--------------------------------------------------------------------------

  template std::optional<H1FEntry> BookStoreManager::getObject<Hist1F>(
    const Processor*, const std::filesystem::path&, const std::string_view&) ;
  
  H1FEntry BookStoreManager::getHist1F(
    const Processor *proc,
    const std::filesystem::path &path,
    const std::string_view &name  ) {
    auto res = getObject<Hist1F>(proc, path, name);
    if ( res ) return std::move(res.value());
    MARLIN_THROW (" try to access not existing histogram!");
  }


  //--------------------------------------------------------------------------
  
  
  H1FEntry BookStoreManager::bookHist1F (
    const Processor *proc,
    const std::filesystem::path &path,
    const std::string_view &name,
    const std::string_view &title,
    const AxisConfigD &axisconfig,
    const BookFlag &flag) {

    std::optional<H1FEntry> res = getObject<Hist1F>(proc, path, name) ;
    if ( res ) {
      const book::EntryKey& key = res.value().key();
      if (   key.flags != flag 
          || key.type !=  std::type_index(typeid(Hist1F))) {
        MARLIN_THROW("try to book to the same spot again");
      } 
      return std::move(res.value());
    }



    std::filesystem::path absPath = constructPath( proc, path ) ;

    book::EntryData<Hist1F> data(title, axisconfig);

    if( flag.contains(book::Flags::Book::MultiCopy)) {
      return _bookStore.book( path, name, data.multiCopy(_application->getConcurrency()) ) ;
    } else if ( flag.contains(book::Flags::Book::MultiShared)) {
      return _bookStore.book( path, name, data.multiShared() ) ;
    }

    return _bookStore.book( path, name, data.single() ) ;
  }
  
  //--------------------------------------------------------------------------
  
  template<typename T>
  std::optional<book::Handle<book::Entry<T>>> BookStoreManager::getObject(
    const Processor *proc,
    const std::filesystem::path &path,
    const std::string_view &name ) {

    book::Selection res = _bookStore.find(
      book::ConditionBuilder()
        .setType(typeid(T))
        .setPath(constructPath(proc, path).string()) // TODO: life time check
        .setName(name) ) ;
    if( res.size() != 1 ) {
      _logger->log<DEBUG>() << "Failed search for object!";
      return std::nullopt;
    }
    return std::optional(res.get(0).handle<Hist1F>());
  }

  //--------------------------------------------------------------------------
  
  std::filesystem::path BookStoreManager::constructPath(
    const Processor *proc,
    const std::filesystem::path &path ) {
    return std::filesystem::path(proc->name()) / path;
  }
}
