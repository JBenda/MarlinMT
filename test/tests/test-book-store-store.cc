#include "TDirectory.h"
#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"
#include "TKey.h"
#include <UnitTesting.h>
#include <cstring>
#include <filesystem>
#include <type_traits>

#include "marlin/book/BookStore.h"
#include "marlin/book/Handle.h"
#include "marlin/book/Hist.h"
#include "marlin/book/Serializer.h"

using namespace marlin::book ;
using namespace marlin::book::types ;

std::string expected = "expected: " ;



template < typename T >
struct HDetails {} ;

template <>
struct HDetails< H1F > {
  using RootT                  = TH1F ;
  static constexpr char Name[] = "H1F" ;
} ;
template <>
struct HDetails< H1D > {
  using RootT                  = TH1D ;
  static constexpr char Name[] = "H1D" ;
} ;
template <>
struct HDetails< H1I > {
  using RootT                  = TH1I ;
  static constexpr char Name[] = "H1I" ;
} ;
template <>
struct HDetails< H2F > {
  using RootT                  = TH2F ;
  static constexpr char Name[] = "H2F" ;
} ;
template <>
struct HDetails< H2D > {
  using RootT                  = TH2D ;
  static constexpr char Name[] = "H2D" ;
} ;
template <>
struct HDetails< H2I > {
  using RootT                  = TH2I ;
  static constexpr char Name[] = "H2I" ;
} ;
template <>
struct HDetails< H3F > {
  using RootT                  = TH3F ;
  static constexpr char Name[] = "H3F" ;
} ;
template <>
struct HDetails< H3D > {
  using RootT                  = TH3D ;
  static constexpr char Name[] = "H3D" ;
} ;
template <>
struct HDetails< H3I > {
  using RootT                  = TH3I ;
  static constexpr char Name[] = "H3I" ;
} ;

template < typename T >
struct Bin {
  Bin() {}
  Bin( const T & ){} ;
} ;

template <int N, typename T >
class HistTest {
  using Type                        = marlin::book::types::HistT<HistConfig<double, T, N>>;
  using RootT                       = typename HDetails< Type >::RootT ;
  static constexpr const char *Name = HDetails< Type >::Name ;

  void CheckAxis( TAxis *axis, const AxisConfig<double> &config ) const {
    if ( std::string(config.title()) !=  axis->GetTitle() ) {
      throw expected + _path.string() + " to have axis with title: '"
        + config.title() + "' but found: '" + axis->GetTitle() + "'" ;
    }
  }

public:
  HistTest( const std::string_view            name,
            const std::string_view            title,
            const std::array< AxisConfig<double>, N > config )
    : _name{std::move( name )}, _title{std::move( title )}, _config{std::move(
                                                              config )} {}
  const std::string_view &name() const { return _name; }
  void                    Test( TKey *key ) const {
    RootT *hist = key->ReadObject< RootT >() ;
    if ( hist == nullptr ) {
      throw expected + _path.string() + " to be an " + Name + ", but it isn't" ;
    }
    if ( _title != static_cast< std::string_view >( hist->GetTitle() ) ) {
      throw expected + _path.string() + " to have title: '" + _title
        + "' but found '" + hist->GetTitle() + "'!" ;
    }

    CheckAxis( hist->GetXaxis(), _config[0] ) ;
    if constexpr ( N > 1 ) {
      CheckAxis( hist->GetYaxis(), _config[1] ) ;
    }
    if constexpr ( N > 2 ) {
      CheckAxis( hist->GetZaxis(), _config[2] ) ;
    }
  }

  void Create( BookStore &store, const std::filesystem::path &path ) {
    static_assert( N > 0 && N < 4,
                   "only support 1, 2 and 3 dimensional Histograms" ) ;
    _path = path / _name ;
    if constexpr ( N == 1 ) {
      store.book( path / "",
                  std::string( _name ),
                  EntryData< Type >( _title, _config[0] ).single() ) ;
    } else if constexpr ( N == 2 ) {
      store.book(
        path / "",
        std::string( _name ),
        EntryData< Type >( _title, _config[0], _config[1] ).single() ) ;
    } else if constexpr ( N == 3 ) {
      store.book(
        path / "",
        std::string( _name ),
        EntryData< Type >( _title, _config[0], _config[1], _config[2] )
          .single() ) ;
    }
  }

private:
  std::filesystem::path       _path{"-"} ;
  std::string_view            _name{"-"} ;
  std::string_view            _title{"-"} ;
  std::array< AxisConfig<double>, N > _config ;
} ;

template < class TEST, class... Rest >
struct TestDirectoryItr {

  TestDirectoryItr( const TEST &test, const Rest &... rest )
    : _test{test}, _itr{rest...} {}

  void Test( TDirectory *dir ) const {
    TKey *key = dir->GetKey( std::string( _test.name() ).c_str() ) ;
    if ( key == nullptr ) {
      throw expected + _path.string() + " to be exist, but it doesn't" ;
    }
    _test.Test( key ) ;
  }

  void Create( BookStore &store, const std::filesystem::path &path ) {
    _path = path / _test.name() ;
    _test.create( store, path ) ;
  }

private:
  std::filesystem::path             _path{"-"} ;
  TEST                              _test ;
  const TestDirectoryItr< Rest... > _itr ;
} ;

template < class TEST >
struct TestDirectoryItr< TEST > {
  TestDirectoryItr( const TEST &test ) : _test{test} {}
  void Test( TDirectory *dir ) const {
    TKey *key = dir->GetKey( std::string( _test.name() ).c_str() ) ;
    if ( key == nullptr ) {
      throw expected + _path.string() + " to be exist, but it doesn't" ;
    }
    _test.Test( key ) ;
  }
  void Create( BookStore &store, const std::filesystem::path &path ) {
    _path = path / _test.name() ;
    _test.Create( store, path ) ;
  }

private:
  std::filesystem::path _path{"-"} ;
  TEST                  _test ;
} ;

template < class... Tests >
struct TestDirectory {
  TestDirectory( const std::string_view &name, const Tests &... tests )
    : _name{name}, _itr{tests...} {}

  const std::string_view &name() const { return _name; }
  void                    Test( TKey *key ) const {
    TDirectory *dir = key->ReadObject< TDirectory >() ;
    if ( dir == nullptr ) {
      throw expected + _path.string() + " to be an Directory, but it isn't!" ;
    }
    _itr.Test( dir ) ;
  }
  void Create( BookStore &store, const std::filesystem::path &path ) {
    std::filesystem::path nPath = path / name() ;
    _path                       = nPath ;
    _itr.Create( store, nPath ) ;
  }

private:
  const std::string_view       _name{"-"} ;
  TestDirectoryItr< Tests... > _itr ;
  std::filesystem::path        _path{"-"} ;
} ;

template < class... Tests >
struct RootTest {
  RootTest( const Tests &... tests ) : _itr{tests...} {}
  void Test( TFile *file ) const {
    _itr.Test( static_cast< TDirectory * >( file ) ) ;
  } ;

  void Create( BookStore &store ) {
    std::filesystem::path path = "/" ;
    _itr.Create( store, path ) ;
  } ;

private:
  TestDirectoryItr< Tests... > _itr ;
} ;

int main( int /*argc*/, char * /*argv*/[] ) {
  marlin::test::UnitTest test( " BookStore: write to file " ) ;

  AxisConfig<double> axis = {"x", 2, -2.f, 5.f} ;

  RootTest bluePrint(
      TestDirectory("path",
        HistTest< 1, float >( "hist_1", "title_1", {axis} ) 
      )
    ) ;
  std::filesystem::path pathRootFile( "./test.root" ) ;

  BookStore store{} ;
  try {
    bluePrint.Create( store ) ;
  } catch ( const marlin::BookStoreException &expc ) {
    test.test( std::string( "unexpected error: '" ) + expc.what() + "'",
               false ) ;
  }

  Root6SerializerStore ser( pathRootFile ) ;
  store.store( ser ) ;

  TFile *file = TFile::Open( pathRootFile.string().c_str(), "READ" ) ;
  std::optional< std::string > error = std::nullopt ;
  try {
    bluePrint.Test( file ) ;
  } catch ( const std::string &msg ) {
    error = msg ;
  }

  if ( std::filesystem::exists( pathRootFile ) ) {
    std::filesystem::remove(pathRootFile) ;
  }

  test.test( std::string( "Write to ROOT-6 File" )
               + ( error ? ( ": '" + error.value() + "'" ) : "" ),
             !error ) ;

  return 0 ;
}
