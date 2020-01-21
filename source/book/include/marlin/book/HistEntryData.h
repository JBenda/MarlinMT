#pragma once

// -- std includes
#include <array>
#include <functional>

// -- MarlinBook includes
#include "marlin/book/Entry.h"
#include "marlin/book/EntryData.h"
#include "marlin/book/Types.h"

// -- Hist includes
#include "marlin/book/HistEntryDataBase.h"

namespace marlin {
  namespace book {

    /**
     *  @brief EntryData for 1 dimensional Histograms.
     */
    template < typename Config>
    class EntryData< types::HistT<Config>, 0 >
      : public EntryDataBase< types::HistT<Config> > {
      using Type = types::HistT<Config>;
      static constexpr int D = Type::Dimension;
    public:
      /**
       *  @brief Constructor without Title.
       *  @param axis configuration.
       */
      explicit EntryData( const typename Type::AxisConfig_t &axis ) ;

      /**
       *  @brief Constructor.
       *  @param title of the Histogram.
       *  @param axis configuration.
       */
      EntryData( const std::string_view&  title,
                 const typename Type::AxisConfig_t &axis ) ;

      /**
       *  @brief Constructor without title.
       *  @param x_axis configuration of first axis.
       *  @param y_axis configuration of second axis.
       */
      EntryData( const typename Type::AxisConfig_t &x_axis,
                 const typename Type::AxisConfig_t &y_axis ) ;

      /**
       *  @brief Constructor.
       *  @param title of the Histogram.
       *  @param x_axis configuration of first axis.
       *  @param y_axis configuration of second axis.
       */
      EntryData( const std::string_view &  title,
                 const typename Type::AxisConfig_t &x_axis,
                 const typename Type::AxisConfig_t &y_axis ) ;

      /**
       *  @brief Constructor without title.
       *  @param x_axis configuration of first axis.
       *  @param y_axis configuration of second axis.
       *  @param z_axis configuration of third axis.
       */
      EntryData( const typename Type::AxisConfig_t &x_axis,
                 const typename Type::AxisConfig_t &y_axis,
                 const typename Type::AxisConfig_t &z_axis ) ;

      /**
       *  @brief Constructor.
       *  @param title of Histogram
       *  @param x_axis configuration of first axis.
       *  @param y_axis configuration of second axis.
       *  @param z_axis configuration of third axis.
       */
      EntryData( const std::string_view &  title,
                 const typename Type::AxisConfig_t &x_axis,
                 const typename Type::AxisConfig_t &y_axis,
                 const typename Type::AxisConfig_t &z_axis ) ;

      /**
       *  @brief Constructor.
       *  @param title of Histogram
       *  @param axes configurations for [1|2|3] ax[i|e]s 
       */
      EntryData( const std::string_view& title,
                 const std::array<const types::AxisConfig<
                  typename Config::Precision_t>*
                  , Config::Dimension>& axes) ;

    } ;



    /**
     *  @brief EntryData for objects in Single mode.
     */
    template < typename Config >
    class EntryData< types::HistT<Config>, Flags::value( Flags::Book::Single ) > {
      using Object_t = types::HistT<Config>;
      friend EntryDataBase< Object_t > ;
      friend BookStore ;
      static constexpr int D = Object_t::Dimension;

      explicit EntryData(
        const EntryDataBase< Object_t > &data )
        : _data{data} {}

      /**
       *  @brief book Histogram in Single Mode. Only available for 1D Hist.
       *  @param store to where book Histogram.
       */
      template < typename... Args_t, int d = D >
      std::enable_if_t< d == 1, std::shared_ptr< details::Entry > >
      book( BookStore &store, const Args_t &... args ) const ;

      /**
       *  @brief book Histogram in Single Mode. Only available for 2D Hist.
       *  @param store to where book Histogram.
       */
      template < typename... Args_t, int d = D >
      std::enable_if_t< d == 2, std::shared_ptr< details::Entry > >
      book( BookStore &store, const Args_t &... args ) const ;

      /**
       *  @brief book Histogram in Single Mode. Only available for 3D Hist.
       *  @param store to where book Histogram.
       */
      template < typename... Args_t, int d = D >
      std::enable_if_t< d == 3, std::shared_ptr< details::Entry > >
      book( BookStore &store, const Args_t &... args ) const ;

      const EntryDataBase< Object_t > &_data ;
    } ;

    /**
     *  @brief EntryData for objects in MultiCopy Mode
     */
    template < typename Config>
    class EntryData< types::HistT<Config>, Flags::value( Flags::Book::MultiCopy ) > {
      using Object_t = types::HistT<Config>;
      friend EntryDataBase< Object_t > ;
      friend BookStore ;
      static constexpr int D = Object_t::Dimension;

      EntryData( const EntryDataBase< Object_t > &data,
                 std::size_t                                           n )
        : _data{data}, _n{n} {}

      /**
       *  @brief book Histogram in MultiCopy Mode. Only available for 1D Hist.
       *  @param store to where book Histogram.
       */
      template < typename... Args_t, int d = D >
      std::enable_if_t< d == 1, std::shared_ptr< details::Entry > >
      book( BookStore &store, const Args_t &... args ) const ;

      /**
       *  @brief book Histogram in MultiCopy Mode. Only available for 2D Hist.
       *  @param store to where book Histogram.
       */
      template < typename... Args_t, int d = D >
      std::enable_if_t< d == 2, std::shared_ptr< details::Entry > >
      book( BookStore &store, const Args_t &... args ) const ;

      /**
       *  @brief book Histogram in MultiCopy Mode. Only available for 3D Hist.
       *  @param store to where book Histogram.
       */
      template < typename... Args_t, int d = D >
      std::enable_if_t< d == 3, std::shared_ptr< details::Entry > >
      book( BookStore &store, const Args_t &... args ) const ;

      const EntryDataBase< Object_t > &_data ;
      const std::size_t                                     _n ;
    } ;

    /**
     *  @brief  EntryData for objects in MultiShared mode
     */
    template < typename Config>
    class EntryData< types::HistT<Config>, Flags::value( Flags::Book::MultiShared ) > {
      using Object_t = types::HistT<Config>;
      friend EntryDataBase< Object_t > ;
      friend BookStore ;
      static constexpr int D = Object_t::Dimension;

      explicit EntryData(
        const EntryDataBase< Object_t > &data )
        : _data{data} {}

      /**
       *  @brief book Histogram in MultiCopy Mode. Only available for 1D Hist.
       *  @param store to where book Histogram.
       */
      template < typename... Args_t, int d = D >
      std::enable_if_t< d == 1, std::shared_ptr< details::Entry > >
      book( BookStore &store, const Args_t &... args ) const ;

      /**
       *  @brief book Histogram in MultiCopy Mode. Only available for 2D Hist.
       *  @param store to where book Histogram.
       */
      template < typename... Args_t, int d = D >
      std::enable_if_t< d == 2, std::shared_ptr< details::Entry > >
      book( BookStore &store, const Args_t &... args ) const ;

      /**
       *  @brief book Histogram in MultiCopy Mode. Only available for 3D Hist.
       *  @param store to where book Histogram.
       */
      template < typename... Args_t, int d = D >
      std::enable_if_t< d == 3, std::shared_ptr< details::Entry > >
      book( BookStore &store, const Args_t &... args ) const ;

      const EntryDataBase< Object_t > &_data ;
    } ;

  } // end namespace book
} // end namespace marlin
