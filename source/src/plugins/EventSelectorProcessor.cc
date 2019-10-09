
// -- marlin headers
#include <marlin/Processor.h>
#include <marlin/PluginManager.h>
#include <marlin/EventExtensions.h>

// -- lcio headers
#include <lcio.h>

// -- std headers
#include <set>
#include <map>

namespace marlin {

  /** Simple event selector processor. Returns true if the given event
   *  was specified in the EvenList parameter.
   *
   *  <h4>Output</h4>
   *  returns true or false
   *
   * @param  EventList:   pairs of: EventNumber RunNumber
   *
   * @author F. Gaede, DESY
   * @version $Id:$
   */
  class EventSelectorProcessor : public Processor {
    using EventNumberSet = std::set< std::pair< int, int > > ;

   public:
    /**
     *  @brief  Constructor
     */
    EventSelectorProcessor() ;

    // from Processor
    void init() ;
    void processEvent( EVENT::LCEvent * evt ) ;

   protected:
     Property<std::vector<int>> _evtList {this, "EventList",
              "event list - pairs of Eventnumber RunNumber" } ;
    ///< The event list as a set
    EventNumberSet        _evtSet {} ;
  };

  //--------------------------------------------------------------------------
  //--------------------------------------------------------------------------

  EventSelectorProcessor::EventSelectorProcessor() :
    Processor("EventSelector") {
    // modify processor description
    _description = "EventSelectorProcessor returns true if given event was specified in EventList" ;
  }

  //--------------------------------------------------------------------------

  void EventSelectorProcessor::init() {
    // usually a good idea to
    printParameters() ;
    unsigned int nEvts = _evtList.size() ;
    if( nEvts % 2 != 0 ) {
      throw Exception( "EventSelectorProcessor: event list size should be even (list of run / event ids)" ) ;
    }
    for( unsigned i=0 ; i <nEvts ; i+=2 ) {
      _evtSet.insert( std::make_pair( _evtList[i] , _evtList[ i+1 ] ) ) ;
    }
  }

  //--------------------------------------------------------------------------

  void EventSelectorProcessor::processEvent( EVENT::LCEvent * evt ) {
    auto conditions = evt->runtime().ext<ProcessorConditions>() ;
    // if no events specified - always return true
    if( _evtList.size() == 0 ) {
      conditions->set( this, true ) ;
      return ;
    }
    auto iter = _evtSet.find( std::make_pair( evt->getEventNumber() , evt->getRunNumber() ) ) ;
    const bool isInList = (iter != _evtSet.end() ) ;
    //-- note: this will not be printed if compiled w/o MARLINDEBUG=1 !
    log<DEBUG>() << "   processing event: " << evt->getEventNumber()
  		       << "   in run:  " << evt->getRunNumber()
  		       << " - in event list : " << isInList
  		       << std::endl ;
    conditions->set( this, isInList ) ;
  }

  // plugin declaration
  MARLIN_DECLARE_PROCESSOR( EventSelectorProcessor )
}
