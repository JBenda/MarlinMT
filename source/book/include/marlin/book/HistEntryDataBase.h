#pragma  once

// -- std includes
#include <array>

// -- MarlinBook includes
#include  "marlin/book/EntryData.h"
#include  "marlin/book/Types.h"

namespace marlin {
  namespace book {
    
    /**
     *  @brief Base for Histogram EntryData.
     */
    template < class Config >
    class EntryDataBase< types::HistT<Config>>
      : public EntryDataBase< void > {
      template < typename, unsigned long long >
      friend class EntryData ;
      static constexpr std::string_view empty{""} ;
    protected:

      using Type = types::HistT<Config>;
      static constexpr int D = Type::Dimension;
    public:

      explicit EntryDataBase( const std::string_view &title = empty ) ;

      /**
       *  @brief construct EntryData for single booking.
       */
      EntryData< Type, Flags::value( Flags::Book::Single ) >
      single() const ;

      /**
       *  @brief construct EntryData for multi copy booking.
       *  @param n number of memory instances which should be constructed
       */
      [[nodiscard]] EntryData< Type,  Flags::value( Flags::Book::MultiCopy ) >
      multiCopy( std::size_t n ) const ;

      /**
       *  @brief construct EntryData for multi shared booking.
       */
      [[nodiscard]] EntryData< Type, Flags::value( Flags::Book::MultiShared ) >
      multiShared() const ;

    protected:

      /**
       *  @brief read access for passed title.
       */
      [[nodiscard]] const std::string_view &title() const { return _title; }

      /**
       *  @brief access Histogram axis configuration.
       *  @param id of axis, start by 0.
       */
      [[nodiscard]] std::unique_ptr< typename Type::AxisConfig_t > &
      axis( const std::size_t id ) {
        return _axis.at( id ) ;
      }

      /**
       *  @brief access Histogram axis configuration. Can't Modified.
       *  @param id of axis, start by 0.
       */
      [[nodiscard]] const std::unique_ptr< typename Type::AxisConfig_t > &
      axis( const std::size_t id ) const {
        return _axis.at( id ) ;
      }

    private:
      /// Histogram custom title, length = 0 when nothing setted.
      const std::string_view &_title ;
      /// Histogram axis configuration.
      std::array< std::unique_ptr< typename Type::AxisConfig_t >, D > _axis{} ;
    } ;
  } // end namespace book
} // end namespace marlin
