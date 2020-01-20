#include <marlin/ProcessorApi.h>

// -- std headers
#include <utility>

// -- marlin headers
#include <marlin/Exceptions.h>
#include <marlin/EventExtensions.h>



namespace marlin {

  
  H1FEntry ProcessorApi::Book::bookHist1F (
    const Processor *proc, 
    const std::filesystem::path &path, 
    const std::string_view &name,
    const std::string_view &title,
    const AxisConfigD &axisconfig,
    const BookFlag &flags )  {
    return proc->app().bookStoreManager().bookHist1F(
      proc,
      path,
      name,
      title,
      axisconfig,
      flags);
  } 

  //--------------------------------------------------------------------------

  H1FEntry ProcessorApi::Book::getHist1F (
    const Processor *proc,
    const std::filesystem::path &path,
    const std::string_view &name ) {
    return proc->app().bookStoreManager().getHist1F(
        proc, path, name);
  }

  //--------------------------------------------------------------------------
  
  void ProcessorApi::registerForRandomSeeds( Processor *const proc ) {
    proc->app().randomSeedManager().addEntry( proc ) ;
  }

  //--------------------------------------------------------------------------

  unsigned int ProcessorApi::getRandomSeed( const Processor *const proc, EventStore *event ) {
    auto randomSeeds = event->extensions().get<extensions::RandomSeed, RandomSeedExtension>();
    if( nullptr == randomSeeds ) {
      MARLIN_THROW( "No random seed extension in event" ) ;
    }
    return randomSeeds->randomSeed( proc ) ;
  }

  //--------------------------------------------------------------------------

  void ProcessorApi::setReturnValue( const Processor *const proc, EventStore *event, bool value ) {
    auto procConds = event->extensions().get<extensions::ProcessorConditions, ProcessorConditionsExtension>() ;
    if( nullptr == procConds ) {
      MARLIN_THROW( "No processor conditions extension in event" ) ;
    }
    return procConds->set( proc, value ) ;
  }

  //--------------------------------------------------------------------------

  void ProcessorApi::setReturnValue( const Processor *const proc, EventStore *event, const std::string &name, bool value ) {
    auto procConds = event->extensions().get<extensions::ProcessorConditions, ProcessorConditionsExtension>() ;
    if( nullptr == procConds ) {
      MARLIN_THROW( "No processor conditions extension in event" ) ;
    }
    return procConds->set( proc, name, value ) ;
  }

  //--------------------------------------------------------------------------

  bool ProcessorApi::isFirstEvent( EventStore *event ) {
    return *event->extensions().get<extensions::IsFirstEvent, bool>() ;
  }

  //--------------------------------------------------------------------------

  void ProcessorApi::skipCurrentEvent( const Processor *const proc ) {
    proc->log<WARNING>() << "Skipping current event !" << std::endl ;
    MARLIN_SKIP_EVENT( proc ) ;
  }

  //--------------------------------------------------------------------------

  void ProcessorApi::abort( const Processor *const proc, const std::string &reason ) {
    proc->log<WARNING>() << "Stopping application: " << reason << std::endl ;
    MARLIN_STOP_PROCESSING( proc ) ;
  }

}
