// -- marlin headers
#include <marlin/Parameter.h>

namespace marlin {
  
  EParameterType ParameterImpl::type() const {
    return _type ;
  }
  
  //--------------------------------------------------------------------------

  const std::string& ParameterImpl::name() const {
    return _name ;
  }
  
  //--------------------------------------------------------------------------
  
  const std::string& ParameterImpl::description() const {
    return _description ;
  }

  //--------------------------------------------------------------------------

  bool ParameterImpl::isSet() const {
    return _isSet ;
  }
  
  //--------------------------------------------------------------------------
  
  std::string ParameterImpl::str() const {
    return _strFunction() ;
  }
  
  //--------------------------------------------------------------------------
  
  std::string ParameterImpl::typeStr() const {
    return _typeFunction() ;
  }
  
  //--------------------------------------------------------------------------
  
  const std::type_index &ParameterImpl::typeIndex() const {
    return _typeIndex ;
  }
  
  //--------------------------------------------------------------------------
  
  void ParameterImpl::reset() {
    _resetFunction() ;
    _isSet = false ;
  }
  
  //--------------------------------------------------------------------------
  //--------------------------------------------------------------------------
  
  
  void Configurable::checkParameter( const std::string &name ) const {
    if( exists( name ) ) {
      MARLIN_THROW( "Parameter '" + name +  "' already present" ) ;
    }
  }
  
  //--------------------------------------------------------------------------

  bool Configurable::exists( const std::string &name ) const {
    return _parameters.find( name ) != _parameters.end() ;
  }
  
  //--------------------------------------------------------------------------
  
  bool Configurable::isSet( const std::string &name ) const {
    auto iter = _parameters.find( name ) ;
    if( iter == _parameters.end() ) {
      return false ;
    }
    iter->second->isSet() ;
  }
  
  //--------------------------------------------------------------------------

  void Configurable::clear() {
    _parameters.clear() ;
  }
  
  //--------------------------------------------------------------------------
  
  void Configurable::unset() {
    std::for_each( begin(), end(), []( auto &p ){ p.second->reset(); } ) ;
  }
  
  //--------------------------------------------------------------------------
  
  Configurable::iterator Configurable::begin() { 
    return _parameters.begin() ; 
  }
  
  //--------------------------------------------------------------------------
  
  Configurable::const_iterator Configurable::begin() const { 
    return _parameters.begin() ; 
  }    
  
  //--------------------------------------------------------------------------
  
  Configurable::iterator Configurable::end() { 
    return _parameters.end() ; 
  }
  
  //--------------------------------------------------------------------------
  
  Configurable::const_iterator Configurable::end() const { 
    return _parameters.end() ; 
  }
  
}