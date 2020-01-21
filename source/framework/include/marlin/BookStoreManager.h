#pragma once

// -- marlin headers
#include <marlin/MarlinBookConfig.h>
#include <marlin/Logging.h>

// -- std includes
#include <set>

namespace marlin {
  
  // forward declaration
  class Application ;
  class Processor ;

  /**
   *  @brief  BookStoreManager class
   */
  class BookStoreManager {
  public:
    using Logger = Logging::Logger ;
    
  public:
    BookStoreManager() = default;
    BookStoreManager( const BookStoreManager & ) = delete ;
    BookStoreManager &operator=( const BookStoreManager & ) = delete ;
    BookStoreManager( BookStoreManager && ) = delete ;
    BookStoreManager &operator=( BookStoreManager && ) = delete ;
    ~BookStoreManager() ;
    
    /// Whether the book store has been initialized
    [[nodiscard]] bool isInitialized() const ;
    
    /// Initialize the book store manager
    void init( const Application *app ) ;
    
    /**
     *  @brief  Book  a histogram 1D, float type
     *  
     *  @param  proc       the processor booking the histogram
     *  @param  path       the histogram entry path
     *  @param  name       the histogram name
     *  @param  title      the histogram title
     *  @param  axisconfig the histogram X axis config
     *  @param  flags      the book flag policy
     */
    template<typename HistT>
    [[nodiscard]] book::Handle<book::Entry<HistT>> bookHist (
      const std::filesystem::path &path, 
      const std::string_view &name,
      const std::string_view &title,
      const std::array<
        const AxisConfig<typename HistT::Precision_t>*,
        HistT::Dimension> &axesconfig,
      const BookFlag &flags ) ;

    /**
     *  @brief add entry key to write list.
     */
    void addToWrite(const book::EntryKey& key) ;

    /**
     *  @brief remove entry key from write list. 
     */
    void removeFromWrite(const book::EntryKey& key) ;

    /**
     *  @brief receive key from Entry with given path and name.
     *  @param path absolute path of Object
     *  @param name of Object
     *  @return nullptr if path not exist or is not unique
     *  @return EntryKey address else
     */
    const book::EntryKey* getKey(
        const std::filesystem::path &path,
        const std::string_view &name) ;

    /**
     *  @brief access object managed by this store. For internal usage.
     *  @param key key for Entry to Object
     *  @return optional which contains on success a Handle for the Entry for the Object.
     */
    template<typename T>
    [[nodiscard]] std::optional<book::Handle<book::Entry<T>>> getObject( 
        const book::EntryKey *key) ;

    
  private:

    /// The application in which the geometry manager has been initialized
    const Application                   *_application {nullptr} ;
    /// The book store
    book::BookStore                      _bookStore {true} ;
    /// list of entry keys for Entries which should be stored at end of lifetime
    std::set<book::EntryKey>             _entrysToWrite {} ;
    /// The logger instance
    Logger                               _logger {nullptr} ;
    /// path to file to store objects
    std::filesystem::path                _storeFile{""};
  };

  
}
