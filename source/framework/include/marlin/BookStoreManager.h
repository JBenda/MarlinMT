#pragma once

// -- marlin headers
#include <marlin/MarlinBookConfig.h>
#include <marlin/Logging.h>

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
    BookStoreManager() { std::cout << "BSM constructed:!";} ;
    BookStoreManager( const BookStoreManager & ) = delete ;
    BookStoreManager &operator=( const BookStoreManager & ) = delete ;
    BookStoreManager( BookStoreManager && ) = delete ;
    BookStoreManager &operator=( BookStoreManager && ) = delete ;
    ~BookStoreManager() = default ;
    
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
    [[nodiscard]] H1FEntry bookHist1F (
      const Processor *proc, 
      const std::filesystem::path &path, 
      const std::string_view &name,
      const std::string_view &title,
      const AxisConfigD &axisconfig,
      const BookFlag &flags ) ;

    /**
     *  @brief Get handle for booked histogram 1D, float type.
     *
     *  @param proc the processor which booked the histogram
     *  @param path the histogram entry path
     *  @param name the histogram name
     */
    [[nodiscard]] H1FEntry getHist1F (
      const Processor *proc,
      const std::filesystem::path &path,
      const std::string_view &name ) ;

    
  private:
    std::filesystem::path constructPath(
        const Processor *proc,
        const std::filesystem::path &path ) ;

    template<typename T>
    [[nodiscard]] std::optional<book::Handle<book::Entry<T>>> getObject(
      const Processor *proc,
      const std::filesystem::path &path,
      const std::string_view &name ) ;

    /// The application in which the geometry manager has been initialized
    const Application                   *_application {nullptr} ;
    /// The book store
    book::BookStore                      _bookStore {true} ;
    /// The logger instance
    Logger                               _logger {nullptr} ;
  };

  
}
