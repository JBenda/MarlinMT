
// -- marlin headers
#include <marlin/BookStoreManager.h>
#include <marlin/Application.h>
#include <marlin/Exceptions.h>
#include <marlin/Processor.h>

#include <marlin/book/StoreWriter.h>  

namespace marlin {

  BookStoreManager::~BookStoreManager() {
    book::StoreWriter writer ( _storeFile ) ;
    _bookStore.storeList(
      writer,
      _entrysToWrite.begin(), 
      _entrysToWrite.end());
  }

  //--------------------------------------------------------------------------

  void BookStoreManager::init( const Application *app ) {
    if( isInitialized() ) {
      MARLIN_THROW( "BookStoreManager::init: Already initialized !" ) ;
    }
    _application = app ;
    _logger = _application->createLogger( "BookStoreManager" ) ;
    /*_storeFile = app->globalParameters()->getValue<std::string>(
        );*/
    _storeFile = "./out.root";
  }
  
  //--------------------------------------------------------------------------
  
  bool BookStoreManager::isInitialized() const {
    return ( nullptr != _application ) ;
  }

  //--------------------------------------------------------------------------

  template std::optional<H1FEntry> BookStoreManager::getObject<Hist1F>(
    const book::EntryKey*) ;

  template book::Handle<book::Entry<Hist1F>> BookStoreManager::bookHist(
      const std::filesystem::path&,
      const std::string_view&,
      const std::string_view&,
      const std::array<
        const AxisConfig<typename Hist1F::Precision_t>*,
        Hist1F::Dimension>&,
      const BookFlag&) ;

  //--------------------------------------------------------------------------
    
  
  template<typename HistT>
  book::Handle<book::Entry<HistT>> BookStoreManager::bookHist (
    const std::filesystem::path &path,
    const std::string_view &name,
    const std::string_view &title,
    const std::array<
      const AxisConfig<typename HistT::Precision_t>*,
      HistT::Dimension> &axesconfig,
    const BookFlag &flag) {

    using Entry_t = book::Handle<book::Entry<HistT>>;

    bool store = flag.contains(book::Flags::Book::Store);

    BookFlag flagsToPass = flag &
      (   book::Flags::Book::MultiCopy 
        | book::Flags::Book::MultiShared 
        | book::Flags::Book::Single) ;
    std::optional<Entry_t> res = getObject<HistT>(getKey(path, name)) ;
    if ( res ) {
      const book::EntryKey& key = res.value().key();
      if (   key.flags != flagsToPass
          || key.type !=  std::type_index(typeid(Hist1F))) {
        MARLIN_THROW("try to book to the same spot again");
      } 
      return std::move(res.value());
    }

    book::EntryData<HistT> data(title, axesconfig);

    H1FEntry entry;

    if( flag.contains(book::Flags::Book::MultiCopy)) {
      entry =  _bookStore.book( path, name, data.multiCopy(_application->getConcurrency()) ) ;
    } else if ( flag.contains(book::Flags::Book::MultiShared)) {
      entry =  _bookStore.book( path, name, data.multiShared() ) ;
    } else {
      entry =  _bookStore.book( path, name, data.single() ) ;
    }

    if (store) {
      addToWrite( entry.key() ) ;
    }

    return entry;
  }
  
  //--------------------------------------------------------------------------
  
  template<typename T>
  std::optional<book::Handle<book::Entry<T>>> BookStoreManager::getObject(
      const book::EntryKey *key) {
      if (key) {
        if (key->type == std::type_index(typeid(T))) {
          return std::optional(_bookStore.entry<T>(*key));
        } else {
          MARLIN_THROW("try to access Object which wrong type!");
        }
      }
      return std::nullopt;
  }

  //--------------------------------------------------------------------------
  
  void BookStoreManager::addToWrite( const book::EntryKey& key) {
    _entrysToWrite.insert(key);
  }

  //--------------------------------------------------------------------------
  
  void BookStoreManager::removeFromWrite( const book::EntryKey& key) {
    _entrysToWrite.erase(key);
  }

  //--------------------------------------------------------------------------
  
  const book::EntryKey* BookStoreManager::getKey(
    const std::filesystem::path &path,
    const std::string_view &name)
  {
    book::Selection res = _bookStore.find(
      book::ConditionBuilder()
        .setPath(path.string()) // TODO: life time check
        .setName(name) ) ;
    if( res.size() != 1 ) {
      _logger->log<MESSAGE>() << "Failed search for object!";
      return nullptr;
    }
    return &res.get(0).key();
      
  }

}
