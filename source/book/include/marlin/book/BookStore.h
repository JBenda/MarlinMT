#pragma once

// -- std includes
#include <atomic>
#include <filesystem>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>

// -- Marlin includes
// #include "marlin/Exceptions.h"

// -- MarlinBook includes
#include "marlin/book/Condition.h"
#include "marlin/book/Entry.h"
#include "marlin/book/EntryData.h"
#include "marlin/book/Flags.h"
#include "marlin/book/MemLayout.h"
#include "marlin/book/Selection.h"
#include "marlin/book/Types.h"

namespace marlin {
  /// contains classes needed to book and managed booked objects.
  namespace book {

    // -- MarlinBook forward declaration
    class StoreWriter;

    template < typename T >
    class Entry {} ;

    template < typename T >
    class Handle< Entry< T > > {
      friend BookStore ;
      friend WeakEntry ;

      using ThreadId_t = std::thread::id;
      using IdMap_t = std::unordered_map< ThreadId_t, std::size_t > ;

      /// constructor
      explicit Handle( std::shared_ptr< const details::Entry > entry )
        : _entry{std::move( entry )},
          _mapping( std::make_unique< IdMap_t >() ) {}

      /// maps outside id to internal id
      std::size_t unmap( const ThreadId_t& id ) ;

    public:
      Handle()                            = default ;
      /// no copy
      Handle( const Handle & )            = delete ;
      /// no copy
      Handle &operator=( const Handle & ) = delete ;

      ~Handle()                           = default ;

      /// custom move (atomic is not movable)
      Handle( Handle &&hnd ) noexcept ;

      /// custom move (atomic is not movable)
      Handle &operator=( Handle &&hnd ) noexcept ;

      /**
       *  @brief get handle for Object. 
       *  offers handle based on thread_id to avoid unused duplications.
       */
      Handle< T > handle() ;

      const T& merged() const ;

      [[nodiscard]] const EntryKey& key() const {
        return _entry->key();
      }

    private:
      /// reference to handled Entry.
      std::shared_ptr< const details::Entry > _entry{nullptr} ;
      /// maps ids to Handle instances.
      std::unique_ptr< IdMap_t > _mapping{} ;
      /// count number of Handle instances.
      std::atomic< std::size_t > _count{0} ;
      /// shared mutex for serialisation when writing 
      std::shared_mutex _mappingAcces{};
    } ;

    /**
     *  @brief Managed Access and creation of Objects.
     */
    class BookStore {
      template < typename, unsigned long long >
      friend class EntryData ;

      /**
       *  @brief holds references for identify an entry.
       */
      class Identifier {
      public:
        /// Functor for hashing Identifier
        struct Hash {
          std::size_t operator()( const Identifier &id ) const ;
        } ;

        explicit Identifier( std::filesystem::path path)
          : _path{std::move(path)} {}

        bool operator==( const Identifier &id ) const {
          return _path == id._path ;
        }

      private:
        std::filesystem::path _path;
      } ;

      /**
       *  @brief register Entry in store.
       *  generate id for Entry.
       *  @return shared pointer to new Entry
       */
      std::shared_ptr< details::Entry >
      addEntry( const std::shared_ptr< EntryBase > &entry, EntryKey key ) ;

      /**
       *  @brief 
       *  @throw BookStoreException key not exist in Sore
       */
      const std::shared_ptr<details::Entry> 
        &getPtr( const EntryKey &key ) const {
        try {
          return _entries[key.idx] ;
        } catch ( const std::out_of_range& ) {
          MARLIN_BOOK_THROW( "Invalid key." ) ;
        }
      }

      /**
       *  @brief creates an Entry for a default Object.
       *  @tparam T object which should be booked.
       *  @tparam Args_t types of parameters to construct T.
       *  @param path location to store the Entry.
       *  @param name name of the Entry.
       *  @param ctor_p parameters to construct the object.
       */
      template < class T, typename... Args_t >
      std::shared_ptr< details::Entry > bookSingle( 
        std::filesystem::path path,
        Args_t... ctor_p ) ;

      /**
       *  @brief creates an Entry for parallel access.
       *  Creates Multiple copy's of the object to avoid locks.
       *  \see BookStore::book
       *  @note getting the results is triggers a merging →  expensive.
       *  @param n number of instances which should be created.
       *  (max level of pluralism)
       */
      template < class T,
                 void ( *MERGE )( const std::shared_ptr< T > &,
                                  const std::shared_ptr< T > & ),
                 typename... Args_t >
      std::shared_ptr< details::Entry > bookMultiCopy( 
        std::size_t           n,
        std::filesystem::path path,
        Args_t... ctor_p ) ;

      /**
       *  @brief creates an Entry for parallel access.
       *  Creates one object in Memory and modifiers.
       *  \see BookStore::book
       */
      template < class T, typename... Args_t >
      std::shared_ptr< details::Entry > bookMultiShared( 
          std::filesystem::path path,
          Args_t... ctor_p ) ;

      /**
       *  @brief normalize and check path for internal usage. 
       *  @throw BookStoreException if path is no absolute path to a directory.
       *  @param path absolute directory path.
       */
      static std::filesystem::path normalizeDirPath(const std::filesystem::path& path);

      /**
       *  @brief get Entry from key.
       *  @throw BookStoreException key not exist in Store.
       */
      details::Entry &get( std::size_t const idx ) const {
        try {
          return *_entries[idx] ;
        } catch ( const std::out_of_range & ) {
          MARLIN_BOOK_THROW( "Invalid key." ) ;
        }
      }

      /**
       *  @brief get Entry from key.
       *  @throw BookStoreException key not exist in Store.
       */
      details::Entry &get( const EntryKey &key ) const { return get( key.idx ); }


    public:

      explicit BookStore( bool allowMoving = false )
        : _constructThread( std::this_thread::get_id() ), 
          _allowMoving{allowMoving} { }

      /**
       *  @brief book new object.
       *  @param path to store object.
       *  @param name of object.
       *  @note path + name must be unique in the store.
       *  @param data describing access and construction of object.
       *  @throw BookStoreException when:
       *    - object with same path + name already exist
       *    - book was constructed in different thread. \see{_allowMoving}
       */
      template < class T >
      Handle< Entry< typename T::Object_t > >
      book( const std::filesystem::path& path,
            const std::string_view&      name,
            const T                     &data ) ;


      /**
       *  @brief get access to entry from key. 
       */
      template<typename T>
      Handle<Entry<T>> entry(const EntryKey &key ) const ;

      /**
       *  @brief select every Entry which matches the condition.
       *  @return Selection with matches Entries.
       */
      Selection find( const Condition &cond ) const ;

      /**
       *  @brief removes an Entry from BookStore.
       *  the handles are keeping a reference to the result.
       *  @attention modifying removed objects results in undefined behavior.
       */
      void remove( const EntryKey &key ) ;

      /**
       *  @brief removes every Entry from the selection from the BookStore.
       *  calls BookStore::remove(const Entry &e)
       *  for every Entry in Selection.
       */
      void remove( const Selection &selection ) ;

      /**
       *  @brief clears the store.
       *  the handles keep a reference to the result.
       *  @attention modifying removed objects results in undefined behavior.
       */
      void clear() ;

      /**
       *  @brief saves all Objects in one Root-File. 
       *  @param writer used to serialize objects. 
       *  @throw BookStoreException when: Sere kisser 
       *    - directory to store not exist.
       */
      void store( StoreWriter& writer ) const ;
      
      /**
       *  @brief stores only Objects which key is listed.   
       *  @param writer used to serialize objects. 
       *  @param begin of list with keys
       *  @param end of list with keys (not included)
       *  @tparam Itr Iterator type used for traversing keys
       */
      template<typename Itr>
      void storeList( StoreWriter& writer, Itr begin, Itr end) const ; 

      void storeSelection( StoreWriter& writer, const Selection& selection ) const ;

    private:
      /// stores Entries created by BookStore.
      std::vector< std::shared_ptr< details::Entry > > _entries{} ;
      /// stores path+name -> Entry Id
      std::unordered_map< Identifier, std::size_t, Identifier::Hash >
                      _idToEntry{} ;
      std::thread::id _constructThread ;
      /// when false only allow booking from construction thread. Avoid races.
      const bool _allowMoving{false} ;
    } ;

    //--------------------------------------------------------------------------

    template < class T, typename... Args_t >
    std::shared_ptr< details::Entry >
    BookStore::bookSingle( std::filesystem::path path,
                           Args_t...              ctor_p ) {
      EntryKey key{std::type_index( typeid( T ) )} ;
      key.path       = std::move( path ) ;
      key.mInstances = 1 ;
      key.flags      = Flags::Book::Single ;

      auto entry = std::make_shared< EntrySingle< T > >( Context(
        std::make_shared< SingleMemLayout< T, Args_t... > >( ctor_p... ) ) ) ;

      return addEntry( entry, key ) ;
    }

    //--------------------------------------------------------------------------

    template < class T,
               void ( *MERGE )( const std::shared_ptr< T > &,
                                const std::shared_ptr< T > & ),
               typename... Args_t >
    std::shared_ptr< details::Entry >
    BookStore::bookMultiCopy( std::size_t           n,
                              std::filesystem::path path,
                              Args_t... ctor_p ) {
      EntryKey key{std::type_index( typeid( T ) )} ;
      key.path       = std::move( path ) ;
      key.mInstances = n ;
      key.flags      = Flags::Book::MultiCopy ;

      auto entry     = std::make_shared< EntryMultiCopy< T > >(
        Context( std::make_shared< SharedMemLayout< T, MERGE, Args_t... > >(
          n, ctor_p... ) ) ) ;

      return addEntry( entry, key ) ;
    }

    //--------------------------------------------------------------------------

    template < class T, typename... Args_t >
    std::shared_ptr< details::Entry >
    BookStore::bookMultiShared( std::filesystem::path path,
                                Args_t...              ctor_p ) {
      EntryKey key{std::type_index( typeid( T ) )} ;
      key.path       = std::move(path) ;
      key.mInstances = 1 ;
      key.flags      = Flags::Book::MultiShared ;

      auto entry = std::make_shared< EntryMultiShared< T > >( Context(
        std::make_shared< SingleMemLayout< T, Args_t... > >( ctor_p... ) ) ) ;

      return addEntry( entry, key ) ;
    }

    //--------------------------------------------------------------------------

    template < class T >
    Handle< Entry< typename T::Object_t > >
    BookStore::book( const std::filesystem::path &path,
                     const std::string_view      &name,
                     const T                     &data ) {
      std::filesystem::path nPath = normalizeDirPath(path);
      nPath /= name;
      if ( !_allowMoving && std::this_thread::get_id() != _constructThread ) {
        MARLIN_BOOK_THROW( "Booking is only allowed "
                        "from the construction Thread" ) ;
      }

      auto entry = _idToEntry.find( Identifier(nPath)) ;
      if ( entry == _idToEntry.end() ) {
        return Handle< Entry< typename T::Object_t > >(
          data.template book< std::filesystem::path >(
            *this, nPath ) ) ;
      }

      MARLIN_BOOK_THROW(std::string( "Entry path:'" )
                        + static_cast< std::string >( nPath ) + "' name:'"
                        + static_cast< std::string >( name )
                        + "' is already booked!" ) ;
    }

    //--------------------------------------------------------------------------

    template < typename T >
    std::size_t Handle< Entry< T > >::unmap( 
        const Handle<Entry<T>>::ThreadId_t& id ) {
      {
        std::shared_lock lock(_mappingAcces);
        auto itr = _mapping->find( id ) ;
        if ( itr != _mapping->end() ) {
          return itr->second;
        }
      }

      std::unique_lock lock(_mappingAcces);
      return _mapping->insert(std::make_pair( id, _count++ ) ).first->second;
    }

    //--------------------------------------------------------------------------
    
    template< typename T >
    const T& Handle< Entry< T > >::merged() const {
      return _entry->handle<T>(0).merged();     
    }

    //--------------------------------------------------------------------------

    template < typename T >
    Handle< Entry< T > >::Handle( Handle &&hnd ) noexcept
      : _entry( nullptr ), _mapping( nullptr ), _count( hnd._count.load() ) {
      _entry   = hnd._entry ;
      _mapping = std::move( hnd._mapping ) ;

      hnd._entry.reset() ;
    }

    //--------------------------------------------------------------------------

    template < typename T >
    Handle< Entry< T > > &Handle< Entry< T > >::
                            operator=( Handle<Entry<T>> &&hnd ) noexcept {
      _entry   = hnd._entry ;
      _mapping = std::move( hnd._mapping ) ;
      _count       = hnd._count.load() ;

      hnd._entry.reset() ;
      return *this ;
    }

    //--------------------------------------------------------------------------

    template < typename T >
    Handle< T > Handle< Entry< T > >::handle() {
      return _entry->handle< T >( unmap( std::this_thread::get_id()) ) ;
    }

    //--------------------------------------------------------------------------
    
    template < typename Itr >
    void BookStore::storeList( StoreWriter& writer, Itr begin, Itr end) const {
      static_assert(std::is_same_v<
            EntryKey,
            std::remove_cv_t<std::remove_reference_t<decltype(*begin)>>>);
      decltype(_entries) storeList{};
      for( Itr itr = begin; itr != end; ++itr) {
        storeList.push_back(getPtr(*itr)); 
      } 
      storeSelection(
        writer,
        Selection::find(
          storeList.begin(),
          storeList.end(),
          ConditionBuilder())
      );
    }

    //--------------------------------------------------------------------------
    
    template<typename T>
    Handle<Entry<T>> BookStore::entry(const EntryKey &key ) const {
      return Handle<Entry<T>>(getPtr(key))  ;
    }

  } // end namespace book

} // end namespace marlin
