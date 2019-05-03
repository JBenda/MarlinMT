#include <marlin/Sequence.h>

// -- marlin headers
#include <marlin/Processor.h>
#include <marlin/Exceptions.h>
#include <marlin/EventExtensions.h>
#include <marlin/EventModifier.h>

// -- std headers
#include <algorithm>

namespace marlin {
  
  SequenceItem::SequenceItem( std::shared_ptr<Processor> proc, bool lock ) :
    _processor(proc),
    _mutex(lock ? std::make_shared<std::mutex>() : nullptr) {
    if( nullptr != _processor ) {
      throw Exception( "SequenceItem: got a nullptr for processor" ) ;
    }
  }
  
  //--------------------------------------------------------------------------
  
  void SequenceItem::processRunHeader( std::shared_ptr<EVENT::LCRunHeader> rhdr ) {
    if( nullptr != _mutex ) {
      std::lock_guard<std::mutex> lock( *_mutex ) ;
      _processor->processRunHeader( rhdr.get() ) ;
    }
    else {
      _processor->processRunHeader( rhdr.get() ) ;
    }
  }
  
  //--------------------------------------------------------------------------

  void SequenceItem::modifyRunHeader( std::shared_ptr<EVENT::LCRunHeader> rhdr ) {
    auto modifier = dynamic_cast<EventModifier*>( _processor.get() ) ;
    if( nullptr == modifier ) {
      return ;
    }
    if( nullptr != _mutex ) {
      std::lock_guard<std::mutex> lock( *_mutex ) ;
      modifier->modifyRunHeader( rhdr.get() ) ;
    }
    else {
      modifier->modifyRunHeader( rhdr.get() ) ;
    }
  }
  
  //--------------------------------------------------------------------------
  
  SequenceItem::ClockPair SequenceItem::processEvent( std::shared_ptr<EVENT::LCEvent> event ) {
    if( nullptr != _mutex ) {
      clock_t start = clock() ;
      std::lock_guard<std::mutex> lock( *_mutex ) ;
      clock_t start2 = clock() ;
      _processor->processEvent( event.get() ) ;
      _processor->check( event.get() ) ;
      clock_t end = clock() ;
      return ClockPair(end-start, end-start2) ;
    }
    else {
      clock_t start = clock() ;
      _processor->processEvent( event.get() ) ;
      _processor->check( event.get() ) ;
      clock_t end = clock() ;
      return ClockPair(end-start, end-start) ;
    }
  }
  
  //--------------------------------------------------------------------------
  
  SequenceItem::ClockPair SequenceItem::modifyEvent( std::shared_ptr<EVENT::LCEvent> event ) {
    auto modifier = dynamic_cast<EventModifier*>( _processor.get() ) ;
    if( nullptr == modifier ) {
      return ClockPair(0, 0) ;
    }
    if( nullptr != _mutex ) {
      clock_t start = clock() ;
      std::lock_guard<std::mutex> lock( *_mutex ) ;
      clock_t start2 = clock() ;
      modifier->modifyEvent( event.get() ) ;
      clock_t end = clock() ;
      return ClockPair(end-start, end-start2) ;
    }
    else {
      clock_t start = clock() ;
      modifier->modifyEvent( event.get() ) ;
      clock_t end = clock() ;
      return ClockPair(end-start, end-start) ;
    }
  }
  
  //--------------------------------------------------------------------------
  
  std::shared_ptr<Processor> SequenceItem::processor() const {
    return _processor ;
  }
  
  //--------------------------------------------------------------------------
  
  const std::string &SequenceItem::name() const {
    return _processor->name() ;
  }

  //--------------------------------------------------------------------------
  //--------------------------------------------------------------------------

  std::shared_ptr<SequenceItem> Sequence::createItem( std::shared_ptr<Processor> processor, bool lock ) const {
    return std::make_shared<SequenceItem>( processor, lock ) ;
  }
  
  //--------------------------------------------------------------------------
  
  void Sequence::addItem( std::shared_ptr<SequenceItem> item ) {
    auto iter = std::find_if(_items.begin(), _items.end(), [&](std::shared_ptr<SequenceItem> i){
      return (i->processor()->name() == item->processor()->name()) ;
    });
    if( _items.end() != iter ) {
      throw Exception( "Sequence::addItem: processor '" + item->processor()->name() + "' already in sequence" ) ;
    }
    _items.push_back( item ) ;
    _clockMeasures[item->processor()->name()] = ClockMeasure() ;
  }
  
  //--------------------------------------------------------------------------
  
  std::shared_ptr<SequenceItem> Sequence::at( Index index ) const {
    return _items.at( index ) ;
  }
  
  //--------------------------------------------------------------------------
  
  Sequence::SizeType Sequence::size() const {
    return _items.size() ;
  }
  
  //--------------------------------------------------------------------------

  void Sequence::processRunHeader( std::shared_ptr<EVENT::LCRunHeader> rhdr ) {
    for ( auto item : _items ) {
      item->processRunHeader( rhdr ) ;
    }
  }

  //--------------------------------------------------------------------------

  void Sequence::modifyRunHeader( std::shared_ptr<EVENT::LCRunHeader> rhdr ) {
    for ( auto item : _items ) {
      item->modifyRunHeader( rhdr ) ;
    }
  }
  
  //--------------------------------------------------------------------------
  
  void Sequence::processEvent( std::shared_ptr<EVENT::LCEvent> event ) {
    try {
      auto extension = event->runtime().ext<ProcessorConditions>() ;
      for ( auto item : _items ) {
        if ( not extension->check( item->name() ) ) {
          continue ;
        }
        auto clockMeas = item->processEvent( event ) ;
        auto iter = _clockMeasures.find( item->name() ) ;
        iter->second._appClock += clockMeas.first / static_cast<double>( CLOCKS_PER_SEC ) ;
        iter->second._procClock += clockMeas.second / static_cast<double>( CLOCKS_PER_SEC ) ;
        iter->second._counter ++ ;
      }
    }
    catch ( SkipEventException& e ) {
      auto iter = _skipEventMap.find( e.what() ) ;
      if ( _skipEventMap.end() == iter ) {
        _skipEventMap.insert( SkippedEventMap::value_type( e.what() , 1 ) ) ;
      }
      else {
        iter->second ++;
      }
    }
  }
  
  //--------------------------------------------------------------------------

  void Sequence::modifyEvent( std::shared_ptr<EVENT::LCEvent> event ) {
    auto extension = event->runtime().ext<ProcessorConditions>() ;
    for ( auto item : _items ) {
      // check runtime condition
      if ( not extension->check( item->name() ) ) {
        continue ;
      }
      auto clockMeas = item->modifyEvent( event ) ;
      auto iter = _clockMeasures.find( item->name() ) ;
      iter->second._appClock += clockMeas.first / static_cast<double>( CLOCKS_PER_SEC ) ;
      iter->second._procClock += clockMeas.second / static_cast<double>( CLOCKS_PER_SEC ) ;
    }
  }
  
  //--------------------------------------------------------------------------
  
  ClockMeasure Sequence::clockMeasureSummary() const {
    ClockMeasure summary {} ;
    for ( auto t : _clockMeasures ) {
      summary._appClock += t.second._appClock ;
      summary._procClock += t.second._procClock ;
      summary._counter += t.second._counter ;
    }
    return summary ;
  }
  
  //--------------------------------------------------------------------------
  //--------------------------------------------------------------------------
  
  SuperSequence::SuperSequence( std::size_t nseqs ) {
    if( 0 == nseqs ) {
      throw Exception( "SuperSequence: number of sequences must be > 0" ) ;
    }
    _sequences.resize(nseqs) ;
    for( std::size_t i=0 ; i<nseqs ; ++i ) {
      _sequences.at(i) = std::make_shared<Sequence>() ;
    }
  }
  
  //--------------------------------------------------------------------------
  
  void SuperSequence::init( const Application *app ) {
    SequenceItemList items ;
    buildUniqueList( items ) ;
    for( auto item : items ) {
      item->processor()->baseInit( app ) ;
    }
  }
  
  //--------------------------------------------------------------------------
  
  std::shared_ptr<Sequence> SuperSequence::sequence( Index index ) const {
    return _sequences.at( index ) ;
  }

  //--------------------------------------------------------------------------

  SuperSequence::SizeType SuperSequence::size() const {
    return _sequences.size() ;
  }
  
  //--------------------------------------------------------------------------
  
  void SuperSequence::processRunHeader( std::shared_ptr<EVENT::LCRunHeader> rhdr ) {
    SequenceItemList items ;
    buildUniqueList( items ) ;
    for( auto item : items ) {
      item->processRunHeader( rhdr ) ;
    }
  }
  
  //--------------------------------------------------------------------------

  void SuperSequence::modifyRunHeader( std::shared_ptr<EVENT::LCRunHeader> rhdr ) {
    SequenceItemList items ;
    buildUniqueList( items ) ;
    for( auto item : items ) {
      item->modifyRunHeader( rhdr ) ;
    }
  }
  
  //--------------------------------------------------------------------------
  
  void SuperSequence::end() {
    SequenceItemList items ;
    buildUniqueList( items ) ;
    for( auto item : items ) {
      item->processor()->end() ;
    }
  }
  
  //--------------------------------------------------------------------------
  
  void SuperSequence::buildUniqueList( SequenceItemList &items ) const {
    auto nprocs = _sequences.at(0)->size() ;
    for( decltype(nprocs) i=0 ; i<nprocs ; ++i ) {
      for( auto seq : _sequences ) {
        items.insert( seq->at(i) ) ;
      }
    }
  }

}
