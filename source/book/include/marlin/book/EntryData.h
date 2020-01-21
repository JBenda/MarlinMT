#pragma once

// -- std includes
#include  <filesystem>
#include  <memory>
#include  <string>
#include  <typeindex>

// -- MarlinBook includes
#include  "marlin/book/Flags.h"
#include  "marlin/book/Types.h"

namespace marlin {
  namespace book {
    // -- MarlinBook forward declaration
    class MemLayout ;

    /**
     *  @brief Data selection to identify and manage an Entry.
     */
    struct EntryKey {

      /// default constructor.
      EntryKey() = default ;

      /// Construct typed EntryKey.
      explicit EntryKey( const std::type_index &t ) : type{t} {}

      /// virtual Entry path
      std::filesystem::path path{""} ;
      /// number of memory instances
      std::size_t mInstances{0} ;
      /// Type of object stored in Entry.
      std::type_index type{typeid( void )} ;
      /// Status flags from Entry.
      Flag_t flags{} ;
      /// unique number for Entry
      std::size_t idx{std::numeric_limits<std::size_t>::max()} ;
    } ;

    /**
     *  @brief Base Class for Entry Data, for similar behavior.
     */
    template < class T >
    class EntryDataBase {} ;

    template <>
    class EntryDataBase< void > {
    public:
      EntryDataBase()                                   = default ;
      EntryDataBase( const EntryDataBase & )            = delete ;
      EntryDataBase &operator=( const EntryDataBase & ) = delete ;
      EntryDataBase( EntryDataBase<void> && )           = delete ;
      EntryDataBase &operator=( EntryDataBase && )      = delete ;
      ~EntryDataBase()                                  = default ;

      // template<typename ... Args_t>
      // void book(BookStore & store, Args_t... args){
      // }
    } ;

    /**
     * @brief Container for data to construct and setup booked object.
     */
    template < class T, unsigned long long = 0>
    class EntryData : public EntryDataBase< void > {} ;

    /**
     *  @brief Data selection for the Entry to work properly.
     */
    struct Context {

      /// default constructor
      Context() = default ;

      /// constructor
      explicit Context( std::shared_ptr< MemLayout > memLayout )
        : mem{std::move( memLayout )} {}

      /// reference to Memory object. For editing and reading data.
      std::shared_ptr< MemLayout > mem{nullptr} ;
    } ;

    /// Base type for Entries. To avoid void pointer.
    class EntryBase {} ;

  } // end namespace book
} // end namespace marlin

namespace std {
  template<>
  struct less<marlin::book::EntryKey> {
    bool operator()(
      const marlin::book::EntryKey& lh,
      const marlin::book::EntryKey& rh) const 
    {
      return lh.idx < rh.idx;
    }
  };
} // end namespace std

