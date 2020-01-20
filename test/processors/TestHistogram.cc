#include <utility>

// -- marlin headers
#include "marlin/Processor.h"
#include "marlin/ProcessorApi.h"
#include "marlin/PluginManager.h"

#include <EVENT/LCCollection.h>
#include <EVENT/MCParticle.h>

// -- lcio headers
#include "IMPL/LCEventImpl.h"
#include "IMPL/LCRunHeaderImpl.h"

using namespace marlin ;

class TestHistogram : public Processor {
public:
  TestHistogram() ;
  void init() final;
  void processEvent( EventStore * evt) final;
  void end() final;

private:
  H1FEntry _histogram;
};

TestHistogram::TestHistogram() :
  Processor("TestHistogram") {}

void TestHistogram::init() {
  _histogram = ProcessorApi::Book::bookHist1F(
    this,
    "/someWhere/",
    "hit",
    "test histogram",
      book::types::AxisConfig<double>(3, 0., 3.)
  );
}

void TestHistogram::processEvent(EventStore * evt) {
  IMPL::LCEventImpl* event 
    = dynamic_cast<IMPL::LCEventImpl*>(evt->event<EVENT::LCEvent>().get());
  H1FHandle hnd = _histogram.handle();
  try {
    EVENT::LCCollection * coll 
      =  event->getCollection("MCParticle");

    int nHits = coll->getNumberOfElements();

    for(int i = 0; i < nHits; ++i) {
      EVENT::MCParticle* par =
        dynamic_cast<EVENT::MCParticle*>(coll->getElementAt(i));

      hnd.fill({par->getEnergy()}, 1.);
    }
  } catch ( EVENT::Exception& ) {
    streamlog_out(ERROR) << " failed to process event, LC error\n";
  }
}

void TestHistogram::end() {
  streamlog_out(ERROR) << "success\n";  
}

MARLIN_DECLARE_PROCESSOR( TestHistogram )
