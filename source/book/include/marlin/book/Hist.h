#pragma once

// -- MarlinBook includes
#include "marlin/book/Types.h"
#include "marlin/book/BookStore.h"

// -- Hist includes
#include "marlin/book/HistEntry.h"
#include "marlin/book/HistEntryData.h"

namespace marlin {
  namespace book {

    //--------------------------------------------------------------------------
    
    template < typename Config >
    EntryDataBase< types::HistT<Config> >::EntryDataBase(
      const std::string_view &title )
      : _title{title} {}

    //--------------------------------------------------------------------------

    template < typename Config >
    EntryData< types::HistT<Config>, Flags::value(Flags::Book::Single) >
    EntryDataBase< types::HistT<Config> >::single() const {
      return EntryData< types::HistT<Config>,
                        Flags::value(Flags::Book::Single) >( *this ) ;
    }

    //--------------------------------------------------------------------------

    template < typename Config >
    EntryData< types::HistT<Config>, Flags::value(Flags::Book::MultiCopy) >
    EntryDataBase< types::HistT<Config> >::multiCopy( std::size_t n ) const {
      return EntryData< types::HistT<Config>,
                        Flags::value(Flags::Book::MultiCopy) >( *this, n ) ;
    }

    //--------------------------------------------------------------------------

    template < typename Config >
    EntryData< types::HistT<Config>, Flags::value(Flags::Book::MultiShared) >
    EntryDataBase< types::HistT<Config> >::multiShared() const {
      return EntryData< types::HistT<Config>,
                        Flags::value(Flags::Book::MultiShared) >( *this ) ;
    }

    //--------------------------------------------------------------------------

    template<typename  Config>
    EntryData< types::HistT<Config>, 0 >::EntryData( 
        const typename types::HistT<Config>::AxisConfig_t &axis )
      : EntryDataBase< types::HistT<Config> >() {
      static_assert(D == 1, "This is no 1D Hist, therefor it can't be constructed with 1 axis.");
      this->axis(0) = std::make_unique<typename types::HistT<Config>::AxisConfig_t>(axis) ;
    }

    //--------------------------------------------------------------------------

    template < typename Config >
    EntryData< types::HistT<Config>, 0 >::EntryData(
        const std::string_view &title, const typename types::HistT<Config>::AxisConfig_t &axis )
      : EntryDataBase< types::HistT<Config> >( title ) {
      static_assert(D == 1, "This is no 1D Hist, therefor it can't be constructed with 1 axis.");
      this->axis(0) = std::make_unique<typename types::HistT<Config>::AxisConfig_t>(axis) ;
    }

    //--------------------------------------------------------------------------

    template < typename Config >
    EntryData< types::HistT<Config>, 0 >::EntryData(
        const typename types::HistT<Config>::AxisConfig_t &     x_axis,
      const typename types::HistT<Config>::AxisConfig_t &     y_axis )
      : EntryDataBase< types::HistT<Config> >( ) {
      static_assert(D == 2, "This is no 2D Hist, therefor it can't be constructed with 2 axis.");
      this->axis(0) = std::make_unique<typename types::HistT<Config>::AxisConfig_t>(x_axis) ;
      this->axis(1) = std::make_unique<typename types::HistT<Config>::AxisConfig_t>(y_axis) ;
    }

    //--------------------------------------------------------------------------

    template < typename Config >
    EntryData< types::HistT<Config>, 0 >::EntryData(
        const std::string_view &title,
      const typename types::HistT<Config>::AxisConfig_t &     x_axis,
      const typename types::HistT<Config>::AxisConfig_t &     y_axis )
      : EntryDataBase< types::HistT<Config> >( title ) {
      static_assert(D == 2, "This is no 2D Hist, therefor it can't be constructed with 2 axis.");
      this->axis(0) = std::make_unique<typename types::HistT<Config>::AxisConfig_t>(x_axis) ;
      this->axis(1) = std::make_unique<typename types::HistT<Config>::AxisConfig_t>(y_axis) ;
    }

    //--------------------------------------------------------------------------

    template < typename Config >
    EntryData< types::HistT<Config>, 0 >::EntryData( 
        const typename types::HistT<Config>::AxisConfig_t &x_axis,
      const typename types::HistT<Config>::AxisConfig_t &y_axis,
      const typename types::HistT<Config>::AxisConfig_t &z_axis )
      : EntryDataBase< types::HistT<Config> >() {
      static_assert(D == 3, "This is no 3D Hist, therefor it can't be constructed with 3 axis.");
      this->axis(0) = std::make_unique<typename types::HistT<Config>::AxisConfig_t>(x_axis) ;
      this->axis(1) = std::make_unique<typename types::HistT<Config>::AxisConfig_t>(y_axis) ;
      this->axis(2) = std::make_unique<typename types::HistT<Config>::AxisConfig_t>(z_axis) ;
    }

    //--------------------------------------------------------------------------

    template < typename Config >
    EntryData< types::HistT<Config>, 0 >::EntryData(
      const std::string_view &title,
      const typename types::HistT<Config>::AxisConfig_t &     x_axis,
      const typename types::HistT<Config>::AxisConfig_t &     y_axis,
      const typename types::HistT<Config>::AxisConfig_t &     z_axis )
      : EntryDataBase< types::HistT<Config> >( title ) {
      static_assert(D == 3, "This is no 3D Hist, therefor it can't be constructed with 3 axis.");
      this->axis(0) = std::make_unique<typename types::HistT<Config>::AxisConfig_t>(x_axis) ;
      this->axis(1) = std::make_unique<typename types::HistT<Config>::AxisConfig_t>(y_axis) ;
      this->axis(2) = std::make_unique<typename types::HistT<Config>::AxisConfig_t>(z_axis) ;
    }

    //--------------------------------------------------------------------------

    template < typename Config >
    Handle< types::HistT<Config> >::Handle(
      const std::shared_ptr< MemLayout > &                        mem,
      const std::shared_ptr< types::HistT<Config> > &              obj,
      typename Handle< types::HistT<Config> >::FillFn_t     fillFn,
      typename Handle< types::HistT<Config> >::FillNFn_t    fillNFn,
      typename Handle< types::HistT<Config> >::FinalizeFn_t finalFn )
      : BaseHandle< types::HistT<Config> >{mem, obj}, _fillFn{std::move(fillFn)},
        _fillNFn{std::move(fillNFn)}, _finalFn{std::move(finalFn)} {}

    //--------------------------------------------------------------------------

    template < typename Config >
    void Handle< types::HistT<Config> >::fill(
      const typename Handle< types::HistT<Config> >::Point_t &x,
      const typename Handle< types::HistT<Config> >::Weight_t &    w ) {
      _fillFn( x, w ) ;
    }

    //--------------------------------------------------------------------------

    template < typename Config >
    template < typename PointContainer, typename WeightContainer>
    void Handle< types::HistT<Config> >::fillN(
      const PointContainer &x,
      const WeightContainer &w ) {
      // FIXME: only for arrays and vectors
      _fillNFn( &(*x.begin()), &(*x.end()), &(*w.begin()), &(*w.end()) ) ;
    }

    //--------------------------------------------------------------------------

    template < typename Config >
    const types::HistT<Config> &Handle< types::HistT<Config> >::merged() {
      _finalFn() ;
      return BaseHandle< types::HistT<Config> >::merged() ;
    }

    //--------------------------------------------------------------------------

    template < typename Config >
    EntrySingle< types::HistT<Config> >::EntrySingle( Context context )
      : _context{std::move(context)} {}

    template < typename Config >
    Handle< types::HistT<Config> > EntrySingle< types::HistT<Config> >::handle() {
      using Hnd_t = Handle<types::HistT<Config> > ;
      auto hist   = _context.mem->at< Type >( 0 ) ;
      return Handle< Type >(
        _context.mem,
        hist,
        [hist]( const typename Hnd_t::Point_t &x,
                const typename Hnd_t::Weight_t &    w ) { hist->Fill( x, w ); },
        [hist]( const typename Type::Point_t *pFirst,
                const typename Type::Point_t *pLast,
                const typename Type::Weight_t *wFirst,
                const typename Type::Weight_t *wLast) {
          hist->FillN( pFirst, pLast, wFirst, wLast ) ;
        },
        []() {} ) ;
    }

    //--------------------------------------------------------------------------

    template < typename Config >
    EntryMultiCopy< types::HistT<Config> >::EntryMultiCopy(
      Context context )
      : _context{std::move(context)} {}

    template < typename Config >
    Handle< types::HistT<Config> >
    EntryMultiCopy< types::HistT<Config> >::handle( std::size_t idx ) {
      using Hnd_t = Handle< Type > ;
      auto pHist  = _context.mem->at< Type >( idx ) ;
      return Hnd_t(
        _context.mem,
        pHist,
        [pHist]( const typename Hnd_t::Point_t &x,
                 const typename Hnd_t::Weight_t &w ) { pHist->Fill( x, w ); },
        [pHist]( const typename Type::Point_t *pFirst,
                const typename Type::Point_t *pLast,
                const typename Type::Weight_t *wFirst,
                const typename Type::Weight_t *wLast) {
          pHist->FillN( pFirst, pLast, wFirst, wLast ) ;
        },
        []() {} ) ;
    }

    //--------------------------------------------------------------------------
    
    template < typename Config >
    void EntryMultiShared< types::HistT<Config> >::flush() {
      for ( auto &filler : _fillers ) {
        if ( auto ptr = filler.lock() ) {
          ptr->Flush() ;
        }
      }
    }


    //--------------------------------------------------------------------------
    
    template < typename Config >
    EntryMultiShared< types::HistT<Config> >::EntryMultiShared(
      Context context )
      : _context{std::move(context)},
        _fillMgr{
          std::make_shared< types::HistConcurrentFillManager< Config  > >(
            *_context.mem->at< Type >( 0 ) )},
        _fillers( 0 ) {}

    //--------------------------------------------------------------------------

    template < typename Config >
    EntryMultiShared< types::HistT<Config> >::~EntryMultiShared() {
      flush() ;
    }

    //--------------------------------------------------------------------------

    template < typename Config >
    Handle< types::HistT<Config> >
    EntryMultiShared< types::HistT<Config> >::handle() {
      using Hnd_t = Handle<Type> ;
      auto pFiller
        = std::make_shared< types::HistConcurrentFiller< Config > >(
          *_fillMgr ) ;
      _fillers.push_back( pFiller ) ;
      return Hnd_t(
        _context.mem,
        _context.mem->at< Type >( 0 ),
        [pFiller = pFiller]( const typename Hnd_t::Point_t &x,
                             const typename Hnd_t::Weight_t &    w ) {
          pFiller->Fill( x, w ) ;
        },
        [pFiller]( const typename Type::Point_t *pFirst,
                const typename Type::Point_t *pLast,
                const typename Type::Weight_t *wFirst,
                const typename Type::Weight_t *wLast) {
          pFiller->FillN( pFirst, pLast,wFirst, wLast ) ;
        },
        [this]() { this->flush(); } ) ;
    }

    //--------------------------------------------------------------------------

    template < typename Config >
    template < typename... Args_t, int d >
    std::enable_if_t< d == 1, std::shared_ptr<Entry> >
    EntryData<types::HistT<Config>, Flags::value(Flags::Book::Single)>
      ::book( BookStore &store, const Args_t &... args ) const {
      return store.bookSingle< Object_t,
                               const std::string_view &,
                               const typename types::HistT<Config>::AxisConfig_t & >(
        args..., _data.title(), *_data.axis(0) ) ;
    }

    //--------------------------------------------------------------------------
    
    template < typename Config >
    template < typename... Args_t, int d >
    std::enable_if_t< d == 2, std::shared_ptr<Entry>>
    EntryData<types::HistT<Config>, Flags::value(Flags::Book::Single)>
      ::book( BookStore &store, const Args_t &... args ) const {
      return store.bookSingle< Object_t,
                               const std::string_view &,
                               const typename types::HistT<Config>::AxisConfig_t &,
                               const typename types::HistT<Config>::AxisConfig_t & >(
        args..., _data.title(), *_data.axis(0), *_data.axis(1) ) ;
    }

    //--------------------------------------------------------------------------
    
    template < typename Config >
    template < typename... Args_t, int d >
    std::enable_if_t< d == 3, std::shared_ptr<Entry> >
    EntryData<types::HistT<Config>, Flags::value(Flags::Book::Single)>
      ::book( BookStore &store, const Args_t &... args ) const {
      return store.bookSingle< Object_t,
                               const std::string_view &,
                               const typename types::HistT<Config>::AxisConfig_t &,
                               const typename types::HistT<Config>::AxisConfig_t &,
                               const typename types::HistT<Config>::AxisConfig_t & >(
        args...,
        _data.title(),
        *_data.axis(0),
        *_data.axis(1),
        *_data.axis(2)) ;
    }

    //--------------------------------------------------------------------------
    
    template < typename Config >
    template < typename... Args_t, int d >
    std::enable_if_t< d == 1, std::shared_ptr<Entry> >
    EntryData<types::HistT<Config>, Flags::value(Flags::Book::MultiCopy)>
      ::book( BookStore &store, const Args_t &... args ) const {
      return store.bookMultiCopy< Object_t,
                                  &types::add,
                                  const std::string_view &,
                                  const typename types::HistT<Config>::AxisConfig_t & >(
        _n, args..., _data.title(), *_data.axis(0) ) ;
    }

    //--------------------------------------------------------------------------
    
    template < typename Config >
    template < typename... Args_t, int d >
    std::enable_if_t< d == 2, std::shared_ptr<Entry> >
    EntryData<types::HistT<Config>, Flags::value(Flags::Book::MultiCopy)>
      ::book( BookStore &store, const Args_t &... args ) const {
      return store.bookMultiCopy<Object_t,
                                  &types::add,
                                  const std::string_view &,
                                  const typename types::HistT<Config>::AxisConfig_t &,
                                  const typename types::HistT<Config>::AxisConfig_t & >(
        _n, args..., _data.title(), *_data.axis(0), *_data.axis(1) ) ;
    }

    //--------------------------------------------------------------------------
    
    template < typename Config >
    template < typename... Args_t, int d >
    std::enable_if_t< d == 3, std::shared_ptr<Entry> >
    EntryData<types::HistT<Config>, Flags::value(Flags::Book::MultiCopy)>
      ::book( BookStore &store, const Args_t &... args ) const {
      return store.bookMultiCopy< Object_t,
                                  &types::add,
                                  const std::string_view &,
                                  const typename types::HistT<Config>::AxisConfig_t &,
                                  const typename types::HistT<Config>::AxisConfig_t &,
                                  const typename types::HistT<Config>::AxisConfig_t & >(
        _n,
        args...,
        _data.title(),
        *_data.axis(0),
        *_data.axis(1),
        *_data.axis(2) ) ;
    }
    
    //--------------------------------------------------------------------------
    
    template < typename Config >
    template < typename... Args_t, int d >
    std::enable_if_t< d == 1, std::shared_ptr<Entry> >
    EntryData<types::HistT<Config>, Flags::value(Flags::Book::MultiShared)>
      ::book( BookStore &store, const Args_t &... args ) const {
      return store.bookMultiShared< Object_t,
                                    const std::string_view &,
                                    const typename types::HistT<Config>::AxisConfig_t & >(
        args..., _data.title(), *_data.axis(0) ) ;
    }

    //--------------------------------------------------------------------------
    
    template < typename Config >
    template < typename... Args_t, int d >
    std::enable_if_t< d == 2, std::shared_ptr<Entry> >
    EntryData<types::HistT<Config>, Flags::value(Flags::Book::MultiShared)>
      ::book( BookStore &store, const Args_t &... args ) const {
      return store.bookMultiShared< Object_t,
                                    const std::string_view &,
                                    const typename types::HistT<Config>::AxisConfig_t &,
                                    const typename types::HistT<Config>::AxisConfig_t & >(
        args..., _data.title, *_data.axis(0), *_data.axis(1) ) ;
    }

    //--------------------------------------------------------------------------
    
    template < typename Config >
    template < typename... Args_t, int d >
    std::enable_if_t< d == 3, std::shared_ptr<Entry> >
    EntryData<types::HistT<Config>, Flags::value(Flags::Book::MultiShared)>
      ::book( BookStore &store, const Args_t &... args ) const {
      return store.bookMultiShared< Object_t,
                                    const std::string_view &,
                                    const typename types::HistT<Config>::AxisConfig_t &,
                                    const typename types::HistT<Config>::AxisConfig_t &,
                                    const typename types::HistT<Config>::AxisConfig_t & >(
        args...,
        _data.title(),
        *_data.axis(0),
        *_data.axis(1),
        *_data.axis(2) ) ;
    }

  } // end namespace book
} // end namespace marlin
