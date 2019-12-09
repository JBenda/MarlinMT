#include <UnitTesting.h>

#include "marlin/book/BookStore.h"
#include "marlin/book/Handle.h"
#include "marlin/book/Hist.h"

std::string unicStr() {
  static std::size_t num = 0 ;
  return std::to_string( ++num ) ;
}

using namespace marlin::book ;
using namespace marlin::book::types ;

int main( int /*argc*/, char * /*argv*/[] ) {
  marlin::test::UnitTest test( " BookStore: find " ) ;
  constexpr std::size_t  bins         = 3 ;
  constexpr float        min          = -1.F ;
  constexpr float        max          = 5.F ;
  constexpr int          nItrerations = 10 ;
  AxisConfig<double>            axis( "a", bins, min, max ) ;
  try {
    {
      BookStore store{} ;
      store.book( "/path/", "name", EntryData< H1F >( axis ).single() ) ;
      store.book( "/path_2/", "name", EntryData< H1I >( axis ).single() ) ;

      auto selection  = store.find( ConditionBuilder().setName( "name" ) ) ;
      auto selection1 = store.find( ConditionBuilder().setType< H1I >() ) ;
      auto selection2 = store.find( ConditionBuilder().setPath( "/path_2/" ) ) ;
      auto selection3
        = store.find( ConditionBuilder().setPath( std::regex( "path(|_2)" ) ) ) ;

      test.test( "Basic find function BookStore",
                 selection.size() == 2 && selection1.size() == 1
                   && selection2.size() == 1 && selection3.size() == 2
                   && selection1.begin()->key().hash
                        == selection2.begin()->key().hash ) ;

      auto subSelection
        = selection.find( ConditionBuilder().setPath( "/path/" ),
                          Selection::ComposeStrategy::AND ) ;
      auto subSelection1 = store.find( subSelection.condition() ) ;

      test.test( "Sub selection composing AND",
                 subSelection.size() == 1 && subSelection1.size() == 1
                   && subSelection.begin()->key().hash
                        == subSelection1.begin()->key().hash ) ;
    }
    {
      BookStore   store{} ;
      std::string path = std::string( "/" ) + unicStr() + '/' ;
      std::string name ;
      EntryData   config = EntryData< H1I >( axis ) ;
      for ( int i = 0; i < nItrerations; ++i ) {
        name = unicStr() ;
        store.book( path, name, config.single() ) ;
      }

      Selection sel = store.find( ConditionBuilder().setPath( path ) ) ;

      {
        Condition  all  = ConditionBuilder() ;
        std::array selC = {
          sel.find( all ), sel.find( all ), sel.find( all ), sel.find( all )} ;

        auto itr = selC[2].begin() + nItrerations / 2 ;
        for ( int i = nItrerations / 2; i < nItrerations; ++i, ++itr ) {
          selC[0].remove( i ) ;
          selC[2].remove( itr ) ;
        }
        selC[1].remove( nItrerations / 2, nItrerations / 2 ) ;
        selC[3].remove( selC[3].begin() + nItrerations / 2, selC[3].end() ) ;

        bool       equal = true ;
        std::array aItr  = {
          selC[0].begin(), selC[1].begin(), selC[2].begin(), selC[3].begin()} ;
        for ( ; equal && aItr[0] != selC[0].end(); ) {
          for ( int i = 0; i < nItrerations / 2 - 1; ++aItr.at( i++ ) ) {
            if ( i < 3
                 && aItr.at( i )->key().hash != aItr.at( i + 1 )->key().hash ) {
              equal = false ;
              break ;
            }
          }
        }
        test.test( "Remove Elements from selection",
                   selC[0].size() == selC[1].size()
                     && selC[1].size() == selC[2].size()
                     && selC[2].size() == selC[3].size() && equal ) ;
      }
    }
    {
      BookStore store{} ;
      store.book( "/", "name", EntryData< H1I >( axis ).single() ) ;
      store.book( "/path/", "name", EntryData< H1I >( axis ).single() ) ;
      store.book( "/", "other", EntryData< H1I >( axis ).single() ) ;

      Selection sel = store.find( ConditionBuilder().setName( "name" ) ) ;
      Selection rem = store.find( !sel.condition() ) ;

      store.remove( rem ) ;

      Selection sel2 = store.find( sel.condition() ) ;
      Selection rem2 = store.find( rem.condition() ) ;

      bool equal = true ;
      auto itr2  = sel2.begin() ;
      for ( auto itr = sel.begin(); itr != sel.end(); ++itr, ++itr2 ) {
        if ( itr->key().hash != itr2->key().hash ) {
          equal = false ;
          break ;
        }
      }
      if ( sel2.size() != sel.size() ) {
        equal = false ;
      }

      store.clear() ;
      Selection selAll = store.find( ConditionBuilder() ) ;

      test.test( "Remove Elements from store",
                 equal && rem2.size() == 0 && selAll.size() == 0 ) ;
    }
    {
      BookStore                 store{} ;
      Handle< Manager< H1F > > e
        = store.book( "/path/", "my Name", EntryData< H1F >( axis ).single() ) ;
      e.handle( 1 ).fill( {0}, 1 ) ;

      Selection sel    = store.find( ConditionBuilder().setName( "my Name" ) ) ;
      Handle< H1F > h = sel.begin()->handle< H1F >().handle( 1 ) ;
      h.fill( {0}, 1 ) ;

      test.test( "Get booked entry from BookStore",
                 e.handle( 1 ).merged().get().GetBinContent( {0} ) == 2
                   && h.merged().get().GetBinContent( {0} ) == 2 ) ;
    }
  } catch ( const marlin::BookStoreException &excp ) {
    test.test( std::string( "unexpected exception: '" ) + excp.what() + "'",
               false ) ;
  }
  return 0 ;
}
