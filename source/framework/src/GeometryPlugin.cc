#include "marlin/GeometryPlugin.h"

// -- marlin headers
#include "marlin/PluginManager.h"
#include "marlin/Application.h"

namespace marlin {

  GeometryPlugin::GeometryPlugin( const std::string &gtype ) :
    Component("Geometry") {
    setName( gtype ) ;
  }

  //--------------------------------------------------------------------------

  void GeometryPlugin::print() const {
    auto typeidx = typeIndex() ;
    message() << "----------------------------------------------------------" << std::endl ;
    message() << "-- Geometry plugin: " << name() << std::endl ;
    message() << "-- Description: " << description() << std::endl ;
    message() << "-- Handle at: " << handle() << std::endl ;
    message()  << "-- Type index: " << std::endl ;
    message()  << "---- name: " << typeidx.name() << std::endl ;
    message()  << "---- hash: " << typeidx.hash_code() << std::endl ;
    if ( _dumpGeometry ) {
      message() << "-- Geometry dump:" << std::endl ;
      dumpGeometry() ;
      message() << "-- End of geometry dump" << std::endl ;
    }
    message() << "----------------------------------------------------------" << std::endl ;
  }
  
  //--------------------------------------------------------------------------
  
  void GeometryPlugin::initialize() {
    auto &config = application().configuration() ;
    if( config.hasSection("geometry") ) {
      setParameters( config.section("geometry") ) ;
    }
  }

} // namespace marlin
