#include <marlin/Application.h>

// -- marlin headers
#include <marlin/PluginManager.h>
#include <marlin/Utils.h>
#include <marlin/DataSourcePlugin.h>
#include <marlin/XMLParser.h>
#include <marlin/MarlinConfig.h>
#include <marlin/EventExtensions.h>
#include <marlin/IScheduler.h>
#include <marlin/SimpleScheduler.h>
#include <marlin/XMLTools.h>
#include <marlin/EventStore.h>
#include <marlin/RunHeader.h>

// -- std headers
#include <cstring>

using namespace std::placeholders ;

namespace marlin {

  void Application::init( int argc, char **argv ) {
    std::string pname = argv[0] ;
    auto pos = pname.find_last_of("/") ;
    _programName = pname.substr(pos+1) ;
    for ( int i=1 ; i<argc ; i++ ) {
      _arguments.push_back( argv[i] ) ;
    }
    // set the main logger name a bit early for clearer logging
    _loggerMgr.mainLogger()->setName( program() ) ;
    parseCommandLine() ;
    printBanner() ;
    // parse the steering file
    _parser = createParser() ;
    _parser->parse() ;
    // write parsed steering file if global parameter is there
    auto globals = globalParameters() ;
    auto outputSteeringFile = globals->getValue<std::string>( "OutputSteeringFile", "" ) ;
    if ( not outputSteeringFile.empty() ) {
      parser()->write( outputSteeringFile ) ;
    }
    // initialize logging
    _loggerMgr.init( this ) ;
    // initialize BookStore
    _bookStoreManager = std::make_unique<BookStoreManager>();
    _bookStoreManager->init( this ) ;
    // check at this point for a scheduler instance
    if( nullptr == _scheduler ) {
      logger()->log<MESSAGE>() << "No scheduler set. Using SimpleScheduler (single threaded program)" << std::endl ;
      _scheduler = std::make_shared<SimpleScheduler>() ;
    }
    // initialize geometry
    _geometryMgr.init( this ) ;
    // initialize scheduler
    _scheduler->init( this ) ;
    // initialize data source
    auto parameters = dataSourceParameters() ;
    auto dstype = parameters->getValue<std::string>( "DataSourceType" ) ;
    auto &pluginMgr = PluginManager::instance() ;
    _dataSource = pluginMgr.create<DataSourcePlugin>( PluginType::DataSource, dstype ) ;
    if( nullptr == _dataSource ) {
      throw Exception( "Data source of type '" + dstype + "' not found in plugins" ) ;
    }
    _dataSource->init( this ) ;
    // setup callbacks
    _dataSource->onEventRead( std::bind( &Application::onEventRead, this, _1 ) ) ;
    _dataSource->onRunHeaderRead( std::bind( &Application::onRunHeaderRead, this, _1 ) ) ;
    // store processor conditions
    auto activeProcs = activeProcessors() ;
    auto processorConds = processorConditions() ;
    const bool useConditions = ( activeProcs.size() == processorConds.size() ) ;
    if( useConditions ) {
      for( std::size_t i=0 ; i<activeProcs.size() ; ++i ) {
        _conditions[ activeProcs[i] ] = processorConds[i] ;
      }
    }
    _initialized = true ;
  }

  //--------------------------------------------------------------------------

  void Application::run() {
    try {
      _dataSource->readAll() ;
    }
    catch( StopProcessingException &e ) {
      logger()->log<ERROR>() << std::endl
          << " **********************************************************" << std::endl
          << " *                                                        *" << std::endl
          << " *   Stop of EventProcessing requested by processor :     *" << std::endl
          << " *                  "  << e.what()                           << std::endl
          << " *     will call end() method of all processors !         *" << std::endl
          << " *                                                        *" << std::endl
          << " **********************************************************" << std::endl
          << std::endl ;
      throw e ;
    }
    _geometryMgr.clear() ;
    _scheduler->end() ;
    // end() ;
  }

  //--------------------------------------------------------------------------

  void Application::printUsage() const {
    logger()->log<MESSAGE>() << " Usage: " << _programName << " [OPTION] [FILE]..." << std::endl
    << "   runs a " << _programName << " application " << std::endl
    << std::endl
    << " Running the application with a given steering file:" << std::endl
    << "   " << _programName << " steer.xml   " << std::endl
    << std::endl
    << "   " << _programName << " [-h/-?]             \t print this help information" << std::endl
    << "   " << _programName << " -x [steer.xml]      \t print an example steering file to output file file (default: marlin_steer.xml)" << std::endl
    // << "   " << _programName << " -c steer.xml        \t check the given steering file for consistency" << std::endl
    // << "   " << _programName << " -u old.xml new.xml  \t consistency check with update of xml file"  << std::endl
    // << "   " << _programName << " -d steer.xml flow.dot\t create a program flow diagram (see: http://www.graphviz.org)" << std::endl
    << std::endl
    << " Example: " << std::endl
    << " To create a new default steering file from any Marlin application, run" << std::endl
    << "     " << _programName << " -x  mysteer.xml" << std::endl
    // << " and then use either an editor or the MarlinGUI to modify the created steering file " << std::endl
    // << " to configure your application and then run it. e.g. : " << std::endl
    // << "     " << _programName << " mysteer.xml > marlin.out 2>&1 &" << std::endl << std::endl
    << " Dynamic command line options may be specified in order to overwrite individual steering file parameters, e.g.:" << std::endl
    << "     " << _programName << " --datasource.LCIOInputFiles=\"input1.slcio input2.slcio\" --geometry.CompactFile=mydetector.xml" << std::endl
    << "            --MyLCIOOutputProcessor.LCIOWriteMode=WRITE_APPEND --MyLCIOOutputProcessor.LCIOOutputFile=out.slcio steer.xml" << std::endl << std::endl
    << "     NOTE: Dynamic options do NOT work together with " << _programName << " options (-x, -f) nor with the MarlinGUI" << std::endl
<< std::endl ;
  }

  //--------------------------------------------------------------------------

  void Application::parseCommandLine() {
    logger()->log<MESSAGE>() << "Parsing command line ..." << std::endl ;
    _steeringFileName.clear() ;
    _cmdLineOptions.clear() ;
    auto cmdLineArgs = _arguments ;
    if ( cmdLineArgs.empty() ) {
      printUsage() ;
      logger()->log<ERROR>() << "No command line option provided. Expected at least a steering file..." << std::endl ;
      ::exit( 1 ) ;
    }
    auto iter = cmdLineArgs.begin() ;
    // start with dynamic arguments
    while ( cmdLineArgs.end() != iter ) {
      auto arg = *iter ;
      if ( arg.substr( 0, 2 ) == "--" ) {
        auto argVec = StringUtil::split<std::string>( arg.substr( 2 ) , "=" ) ;
        if ( argVec.size() != 2 ) {
          printUsage() ;
          logger()->log<ERROR>() << "*** invalid command line option: " << arg << std::endl ;
          ::exit( 1 ) ;
        }
        auto argKey = StringUtil::split<std::string>( argVec[0] , "." ) ;
        if ( argKey.size() != 2 ) {
          printUsage() ;
          logger()->log<ERROR>() << "*** invalid command line option: " << arg << std::endl ;
          ::exit( 1 ) ;
        }
        _cmdLineOptions[ argKey[0] ][ argKey[1] ] = argVec[1] ;
        logger()->log<MESSAGE9>()
          << "steering file " << argKey[0] << ": "
          << "[ " << argKey[0] << "." << argKey[1] << " ]"
          << " will be OVERWRITTEN with value: [\"" << argVec[1] << "\"]"  << std::endl ;
        iter = cmdLineArgs.erase( iter ) ;
        continue;
      }
      ++iter ;
    }
    // parse remaining arguments
    iter = cmdLineArgs.begin() ;
    while ( cmdLineArgs.end() != iter  ) {
      auto arg = *iter ;
      // print plugin manager content and exit
      if ( arg == "-p" ) {
        PluginManager::instance().dump() ;
        ::exit( 0 ) ;
      }
      // print help and exit
      else if ( arg == "-h" || arg == "-?" ) {
        printUsage() ;
        ::exit( 0 ) ;
      }
      // dump an example steering file to output file
      else if( arg == "-x" ) {
        auto nextarg = std::next( iter ) ;
        std::string fname = "marlin_steer.xml" ;
        if( nextarg != cmdLineArgs.end() ) {
          std::string outname = *nextarg ;
          bool xmlext = (outname.size() >= 4 and
           outname.compare(outname.size() - 4, 4, ".xml") == 0 ) ;
          if( xmlext ) {
            fname = outname ;
          }
        }
        XMLTools::dumpRegisteredProcessors( fname ) ;
        ::exit( 0 ) ;
      }
      // last argument is steering file
      else if( std::next( iter ) == cmdLineArgs.end() ) {
        _steeringFileName = arg ;
      }
      else {
        _filteredArguments.push_back( arg ) ;
      }
      ++iter ;
    }
    if ( _steeringFileName.empty() ) {
      printUsage() ;
      logger()->log<ERROR>() << "No steering file provided ..." << std::endl ;
      ::exit( 1 ) ;
    }
    logger()->log<DEBUG2>() << "Parsing command line ... done" << std::endl ;
  }

  //--------------------------------------------------------------------------

  void Application::printBanner( std::ostream &out ) const {
    out << std::endl ;
    out << "    __  __            _ _       " << std::endl ;
    out << "   |  \\/  | __ _ _ __| (_)_ __  " << std::endl ;
    out << "   | |\\/| |/ _` | '__| | | '_ \\" << std::endl ;
    out << "   | |  | | (_| | |  | | | | | |" << std::endl ;
    out << "   |_|  |_|\\__,_|_|  |_|_|_| |_|" << std::endl ;
    out << std::endl ;
    out << "         Version: " << MARLIN_RELEASE << std::endl ;
    out << std::endl ;
    out << "         LICENCE: GPLv3 " << std::endl ;
    out << "    Copyright (C), Marlin Authors" << std::endl ;
    out << std::endl ;
  }

  //--------------------------------------------------------------------------

  std::shared_ptr<StringParameters> Application::globalParameters () const {
    return (nullptr == _parser) ? nullptr : _parser->getParameters( "Global" ) ;
  }

  //--------------------------------------------------------------------------

  std::shared_ptr<StringParameters> Application::geometryParameters () const {
    return (nullptr == _parser) ? nullptr : _parser->getParameters( "Geometry" ) ;
  }

  //--------------------------------------------------------------------------

  std::shared_ptr<StringParameters> Application::dataSourceParameters () const {
    return (nullptr == _parser) ? nullptr : _parser->getParameters( "DataSource" ) ;
  }

  //--------------------------------------------------------------------------

  std::shared_ptr<StringParameters> Application::processorParameters ( const std::string &name ) const {
    return (nullptr == _parser) ? nullptr : _parser->getParameters( name ) ;
  }

  //--------------------------------------------------------------------------

  std::shared_ptr<StringParameters> Application::constants () const {
    return (nullptr == _parser) ? nullptr : _parser->getParameters( "Constants" ) ;
  }

  //--------------------------------------------------------------------------

  std::vector<std::string> Application::activeProcessors () const {
    if( nullptr == _parser ) {
      return std::vector<std::string>() ;
    }
    return _parser->getParameters( "Global" )->getValues<std::string>( "ActiveProcessors", std::vector<std::string>() ) ;
  }

  //--------------------------------------------------------------------------

  std::vector<std::string> Application::processorConditions () const {
    if( nullptr == _parser ) {
      return std::vector<std::string>() ;
    }
    return _parser->getParameters( "Global" )->getValues<std::string>( "ProcessorConditions", std::vector<std::string>() ) ;
  }

  //--------------------------------------------------------------------------

  bool Application::isInitialized() const {
    return _initialized ;
  }

  //--------------------------------------------------------------------------

  Application::Logger Application::logger() const {
    return _loggerMgr.mainLogger() ;
  }

  //--------------------------------------------------------------------------

  Application::Logger Application::createLogger( const std::string &name ) const {
    if ( _loggerMgr.isInitialized() ) {
      return _loggerMgr.createLogger( name ) ;
    }
    else {
      return Logging::createLogger( name ) ;
    }
  }

  //--------------------------------------------------------------------------

  void Application::onEventRead( std::shared_ptr<EventStore> event ) {
    EventList events ;
    while( _scheduler->freeSlots() == 0 ) {
      _scheduler->popFinishedEvents( events ) ;
      if( not events.empty() ) {
        processFinishedEvents( events ) ;
        events.clear() ;
        break;
      }
      std::this_thread::sleep_for( std::chrono::milliseconds(1) ) ;
    }
    // prepare event extensions for users
    // random seeds extension
    auto seeds = _randomSeedMgr.generateRandomSeeds( event.get() ) ;
    auto randomSeedExtension = new RandomSeedExtension( std::move(seeds) ) ;
    event->extensions().add<extensions::RandomSeed>( randomSeedExtension ) ;
    // runtime conditions extension
    auto procCondExtension = new ProcessorConditionsExtension( _conditions ) ;
    event->extensions().add<extensions::ProcessorConditions>( procCondExtension )  ;
    // first event flag
    *( event->extensions().create<extensions::IsFirstEvent, bool>( true ) ) = _isFirstEvent ;
    _isFirstEvent = false ;
    _scheduler->pushEvent( event ) ;
    // check a second time
    _scheduler->popFinishedEvents( events ) ;
    if( not events.empty() ) {
      processFinishedEvents( events ) ;
    }
  }

  //--------------------------------------------------------------------------

  void Application::onRunHeaderRead( std::shared_ptr<RunHeader> rhdr ) {
    logger()->log<MESSAGE9>() << "New run header no " << rhdr->runNumber() << std::endl ;
    _scheduler->processRunHeader( rhdr ) ;
  }

  //--------------------------------------------------------------------------

  const GeometryManager &Application::geometryManager() const {
    return _geometryMgr ;
  }

  //--------------------------------------------------------------------------

  const RandomSeedManager &Application::randomSeedManager() const {
    return _randomSeedMgr ;
  }

  //--------------------------------------------------------------------------

  RandomSeedManager &Application::randomSeedManager() {
    return _randomSeedMgr ;
  }

  //--------------------------------------------------------------------------

  void Application::setScheduler( Scheduler scheduler ) {
    _scheduler = scheduler ;
  }

  //--------------------------------------------------------------------------

  std::shared_ptr<IParser> Application::parser() const {
    return _parser ;
  }

  //--------------------------------------------------------------------------

  std::shared_ptr<IParser> Application::createParser() const {
    auto parser = std::make_shared<XMLParser>( _steeringFileName ) ;
    // tell parser to take into account any options defined on the command line
    parser->setCmdLineParameters( _cmdLineOptions ) ;
    return parser ;
  }

  //--------------------------------------------------------------------------

  void Application::processFinishedEvents( const EventList &events ) const {
    // simple printout for the time being
    for( auto event : events ) {
      logger()->log<MESSAGE9>()
        << "Event uid " << event->uid()
        << " finished" << std::endl ;
    }
  }

  //--------------------------------------------------------------------------
  
  std::size_t Application::getConcurrency() const {
    if ( _concurrency == 0 ) {
      _loggerMgr.mainLogger()->log<ERROR>() 
        << "concurrency is not well defined yet, but already used!";
    }
    return _concurrency;
  }

  //--------------------------------------------------------------------------
  
  std::size_t Application::readConcurrency() {
    auto globals = globalParameters() ;
    auto ccyStr = globals->getValue<std::string>( "Concurrency", "auto" ) ;
    // The concurrency read from the steering file
    std::size_t ccy = (ccyStr == "auto" ?
      std::thread::hardware_concurrency() :
      StringUtil::stringToType<std::size_t>(ccyStr) ) ;
    Logger logger = _loggerMgr.mainLogger();
    logger->log<DEBUG5>() << "-- Application concurrency from steering file " << ccy << std::endl ;
    logger->log<DEBUG5>() << "-- Hardware concurrency: " << std::thread::hardware_concurrency() << std::endl ;
    if ( ccy <= 0 ) {
      logger->log<ERROR>() << "-- Couldn't determine number of threads to use (computed=" << ccy << ")" << std::endl ;
      throw Exception( "Undefined concurrency level" ) ;
    }
    logger->log<MESSAGE>() << "-- Application concurrency set to " << ccy << std::endl ;
    if ( ccy > std::thread::hardware_concurrency() ) {
      logger->log<WARNING>() << "-- Application concurrency higher than the number of supported threads on your machine --" << std::endl ;
      logger->log<WARNING>() << "---- application: " << ccy << std::endl ;
      logger->log<WARNING>() << "---- hardware:    " << std::thread::hardware_concurrency() << std::endl ;
    }
    if ( ccy == 1 ) {
      logger->log<WARNING>() << "-- The program will run on a single thread --" << std::endl ;
      // TODO should we throw here ?? 
      // JBenda: no, else the user needs a different program when he only want's to use 1 core 
    }
    return ccy;
  }
  
  //--------------------------------------------------------------------------
  
  BookStoreManager &Application::bookStoreManager() const {
    return *_bookStoreManager ;
  }

} // namespace marlin
