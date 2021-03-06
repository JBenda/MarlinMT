#include <marlin/concurrency/PEPScheduler.h>

// -- marlin headers
#include <marlin/Application.h>
#include <marlin/Utils.h>
#include <marlin/Sequence.h>
#include <marlin/Processor.h>
#include <marlin/PluginManager.h>
#include <marlin/EventStore.h>
#include <marlin/RunHeader.h>

// -- std headers
#include <exception>
#include <algorithm>
#include <iomanip>
#include <set>

namespace marlin {

  namespace concurrency {

    /**
     *  @brief  ProcessorSequenceWorker class
     */
    class ProcessorSequenceWorker : public WorkerBase<PEPScheduler::InputType,PEPScheduler::OutputType> {
    public:
      using Base = WorkerBase<PEPScheduler::InputType,PEPScheduler::OutputType>;
      using Input = Base::Input ;
      using Output = Base::Output ;

    public:
      ~ProcessorSequenceWorker() = default ;

    public:
      /**
       *  @brief  Constructor
       *
       *  @param  sequence the processor sequence to execute
       */
      ProcessorSequenceWorker( std::shared_ptr<Sequence> sequence ) ;

    private:
      // from WorkerBase<IN,OUT>
      Output process( Input && event ) ;

    private:
      ///< The processor sequence to run in the worker thread
      std::shared_ptr<Sequence>     _sequence {nullptr} ;
    };

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------

    ProcessorSequenceWorker::ProcessorSequenceWorker( std::shared_ptr<Sequence> sequence ) :
      _sequence(sequence) {
      /* nop */
    }

    //--------------------------------------------------------------------------

    ProcessorSequenceWorker::Output ProcessorSequenceWorker::process( Input && event ) {
      Output output {} ;
      output._event = event ;
      try {
        _sequence->processEvent( event ) ;
      }
      catch(...) {
        output._exception = std::current_exception() ;
      }
      return output ;
    }

    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------

    void PEPScheduler::init( Application *app ) {
      _logger = app->createLogger( "PEPScheduler" ) ;
      preConfigure( app ) ;
      configureProcessors( app ) ;
      configurePool() ;
      _startTime = clock::now() ;
    }

    //--------------------------------------------------------------------------

    void PEPScheduler::end() {
      _pool.stop(false) ;
      EventList events ;
      popFinishedEvents( events ) ;
      if( not _pushResults.empty() ) {
        _logger->log<ERROR>() << "This should never happen !!" << std::endl ;
      }
      _logger->log<MESSAGE>() << "Terminating application" << std::endl ;
      _endTime = clock::now() ;
      _superSequence->end() ;
      // print some statistics
      _superSequence->printStatistics( _logger ) ;
      // print additional threading summary
      const auto parallelTime = clock::time_difference( _startTime, _endTime ) - _runHeaderTime ;
      double totalProcessorClock {0.0} ;
      double totalApplicationClock {0.0} ;
      for ( unsigned int i=0 ; i<_superSequence->size() ; ++i ) {
        auto summary = _superSequence->sequence(i)->clockMeasureSummary() ;
        totalProcessorClock += summary._procClock ;
        totalApplicationClock += summary._appClock ;
      }
      const double speedup = totalProcessorClock / parallelTime ;
      const double lockTimeFraction = ((totalApplicationClock - totalProcessorClock) / totalApplicationClock) * 100. ;
      _logger->log<MESSAGE>() << "---------------------------------------------------" << std::endl ;
      _logger->log<MESSAGE>() << "-- Threading summary" << std::endl ;
      _logger->log<MESSAGE>() << "--   N threads:                      " << _superSequence->size() << std::endl ;
      _logger->log<MESSAGE>() << "--   Speedup (serial/parallel):      " << totalProcessorClock << " / " << parallelTime << " = " << speedup << std::endl ;
      if( _superSequence->size() > 1 ) {
        double speedupPercent = (speedup - 1) * 100 / ( _superSequence->size() - 1 ) ;
        if( speedupPercent < 0 ) {
          speedupPercent = 0. ;
        }
        _logger->log<MESSAGE>() << "--   Speedup percentage:             " << speedupPercent << " " << '%' << std::endl ;
      }
      _logger->log<MESSAGE>() << "--   Queue lock time:                " << _lockingTime << " ms" << std::endl ;
      _logger->log<MESSAGE>() << "--   Pop event time:                 " << _popTime << " ms" << std::endl ;
      _logger->log<MESSAGE>() << "--   Lock time fraction:             " << lockTimeFraction << " %" << std::endl ;
      _logger->log<MESSAGE>() << "---------------------------------------------------" << std::endl ;
    }

    //--------------------------------------------------------------------------

    void PEPScheduler::preConfigure( Application *app ) {
      auto globals = app->globalParameters() ;
      auto ccyStr = globals->getValue<std::string>( "Concurrency", "auto" ) ;
      // The concurrency read from the steering file
      std::size_t ccy = (ccyStr == "auto" ?
        std::thread::hardware_concurrency() :
        StringUtil::stringToType<std::size_t>(ccyStr) ) ;
      _logger->log<DEBUG5>() << "-- Application concurrency from steering file " << ccy << std::endl ;
      _logger->log<DEBUG5>() << "-- Hardware concurrency: " << std::thread::hardware_concurrency() << std::endl ;
      if ( ccy <= 0 ) {
        _logger->log<ERROR>() << "-- Couldn't determine number of threads to use (computed=" << ccy << ")" << std::endl ;
        throw Exception( "Undefined concurrency level" ) ;
      }
      _logger->log<MESSAGE>() << "-- Application concurrency set to " << ccy << std::endl ;
      if ( ccy > std::thread::hardware_concurrency() ) {
        _logger->log<WARNING>() << "-- Application concurrency higher than the number of supported threads on your machine --" << std::endl ;
        _logger->log<WARNING>() << "---- application: " << ccy << std::endl ;
        _logger->log<WARNING>() << "---- hardware:    " << std::thread::hardware_concurrency() << std::endl ;
      }
      if ( ccy == 1 ) {
        _logger->log<WARNING>() << "-- The program will run on a single thread --" << std::endl ;
        // TODO should we throw here ??
      }
      // create processor super sequence
      _superSequence = std::make_shared<SuperSequence>(ccy) ;
    }

    //--------------------------------------------------------------------------

    void PEPScheduler::configureProcessors( Application *app ) {
      _logger->log<DEBUG5>() << "PEPScheduler configureProcessors ..." << std::endl ;
      // create list of active processors
      auto activeProcessors = app->activeProcessors() ;
      if ( activeProcessors.empty() ) {
        throw Exception( "PEPScheduler::configureProcessors: Active processor list is empty !" ) ;
      }
      // check for duplicates first
      std::set<std::string> duplicateCheck ( activeProcessors.begin() , activeProcessors.end() ) ;
      if ( duplicateCheck.size() != activeProcessors.size() ) {
        _logger->log<ERROR>() << "PEPScheduler::configureProcessors: the following list of active processors are found to be duplicated :" << std::endl ;
        for ( auto procName : activeProcessors ) {
          auto c = std::count( activeProcessors.begin() , activeProcessors.end() , procName ) ;
          if( c > 1 ) {
            _logger->log<ERROR>() << "   * " << procName << " (" << c << " instances)" << std::endl ;
          }
        }
        throw Exception( "PEPScheduler::configureProcessors: duplicated active processors. Check your steering file !" ) ;
      }
      // populate processor sequences
      for ( size_t i=0 ; i<activeProcessors.size() ; ++i ) {
        auto procName = activeProcessors[ i ] ;
        _logger->log<DEBUG5>() << "Active processor " << procName << std::endl ;
        auto processorParameters = app->processorParameters( procName ) ;
        if ( nullptr == processorParameters ) {
          throw Exception( "PEPScheduler::configureProcessors: undefined processor '" + procName + "'" ) ;
        }
        _superSequence->addProcessor( processorParameters ) ;
      }
      _superSequence->init( app ) ;
      _logger->log<DEBUG5>() << "PEPScheduler configureProcessors ... DONE" << std::endl ;
    }

    //--------------------------------------------------------------------------

    void PEPScheduler::configurePool() {
      // create N workers for N processor sequences
      _logger->log<DEBUG5>() << "configurePool ..." << std::endl ;
      _logger->log<DEBUG5>() << "Number of workers: " << _superSequence->size() << std::endl ;
      for( unsigned int i=0 ; i<_superSequence->size() ; ++i ) {
        _logger->log<DEBUG>() << "Adding worker ..." << std::endl ;
        _pool.addWorker<ProcessorSequenceWorker>( _superSequence->sequence(i) ) ;
      }
      _logger->log<DEBUG5>() << "starting thread pool" << std::endl ;
      // start with a default small number
      _pool.setMaxQueueSize( 2 * _superSequence->size() ) ;
      _pool.start() ;
      _pool.setAcceptPush( true ) ;
      _logger->log<DEBUG5>() << "configurePool ... DONE" << std::endl ;
    }

    //--------------------------------------------------------------------------

    void PEPScheduler::processRunHeader( std::shared_ptr<RunHeader> rhdr ) {
      // Current way to process run header:
      //  - Stop accepting event in thread pool
      //  - Wait for current events processing to finish
      //  - Process run header
      //  - Resume pool access for new event push
      _pool.setAcceptPush( false ) ;
      // need to wait for all current tasks to finish
      // and then process run header
      while( _pool.active() ) {
        std::this_thread::sleep_for( std::chrono::microseconds(10) ) ;
      }
      auto rhdrStart = clock::now() ;
      _superSequence->processRunHeader( rhdr ) ;
      auto rhdrEnd = clock::now() ;
      _runHeaderTime += clock::time_difference<clock::seconds>( rhdrStart, rhdrEnd ) ;
      _pool.setAcceptPush( true ) ;
    }

    //--------------------------------------------------------------------------

    void PEPScheduler::pushEvent( std::shared_ptr<EventStore> event ) {
      // push event to thread pool queue. It might throw !
      auto start = clock::now() ;
      _pushResults.push_back( _pool.push( WorkerPool::PushPolicy::ThrowIfFull, std::move(event) ) ) ;
      _lockingTime += clock::elapsed_since<clock::milliseconds>( start ) ;
    }

    //--------------------------------------------------------------------------

    void PEPScheduler::popFinishedEvents( std::vector<std::shared_ptr<EventStore>> &events ) {
      auto start = clock::now() ;
      auto iter = _pushResults.begin() ;
      while( iter != _pushResults.end() ) {
        const bool finished = (iter->second.wait_for(std::chrono::seconds(0)) == std::future_status::ready) ;
        if( finished ) {
          auto output = iter->second.get() ;
          // if an exception was raised during processing rethrow it there !
          if( nullptr != output._exception ) {
            std::rethrow_exception( output._exception ) ;
          }
          _logger->log<MESSAGE>() << "Finished event uid " << output._event->uid() << std::endl ;
          events.push_back( output._event ) ;
          iter = _pushResults.erase( iter ) ;
          continue;
        }
        ++iter ;
      }
      _popTime += clock::elapsed_since<clock::milliseconds>( start ) ;
    }

    //--------------------------------------------------------------------------

    std::size_t PEPScheduler::freeSlots() const {
      return _pool.freeSlots() ;
    }

  }

} // namespace marlin
