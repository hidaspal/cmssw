#include <memory>
#include <string>
#include <iostream>
#include <fstream>
#include "CondCore/DBOutputService/interface/PoolDBOutputService.h"
#include "CalibTracker/SiPixelLorentzAngle/interface/SiPixelSomeThingDB.h"
#include "DataFormats/SiPixelDetId/interface/PXBDetId.h"
#include "DataFormats/SiPixelDetId/interface/PXFDetId.h"
#include "CondFormats/SiPixelObjects/interface/SiPixelSomeThing.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "Geometry/TrackerGeometryBuilder/interface/PixelGeomDetUnit.h"
#include "Geometry/Records/interface/TrackerDigiGeometryRecord.h"
#include "FWCore/ServiceRegistry/interface/Service.h"


#include "DataFormats/GeometryCommonDetAlgo/interface/MeasurementPoint.h"
#include "DataFormats/GeometryCommonDetAlgo/interface/MeasurementError.h"
#include "DataFormats/GeometrySurface/interface/GloballyPositioned.h"
#include "DataFormats/SiPixelDetId/interface/PixelSubdetector.h"

using namespace std;
using namespace edm;

  //Constructor

SiPixelSomeThingDB::SiPixelSomeThingDB(edm::ParameterSet const& conf) : 
  conf_(conf){
  	magneticField_ = conf_.getParameter<double>("magneticField");
	recordName_ = conf_.getUntrackedParameter<std::string>("record","SiPixelSomeThingRcd");
	useFile_ = conf_.getParameter<bool>("useFile");		
	fileName_ = conf_.getParameter<string>("fileName");

        BPixParameters_ = conf_.getUntrackedParameter<Parameters>("BPixParameters");
        FPixParameters_ = conf_.getUntrackedParameter<Parameters>("FPixParameters");
}

  //BeginJob

void SiPixelSomeThingDB::beginJob(){
  
}
// Virtual destructor needed.

SiPixelSomeThingDB::~SiPixelSomeThingDB() {  

}  

// Analyzer: Functions that gets called by framework every event

void SiPixelSomeThingDB::analyze(const edm::Event& e, const edm::EventSetup& es)
{

	SiPixelSomeThing* SomeThing = new SiPixelSomeThing();
	   
	
	edm::ESHandle<TrackerGeometry> pDD;
	es.get<TrackerDigiGeometryRecord>().get( pDD );
	edm::LogInfo("SiPixelSomeThing") <<" There are "<<pDD->detUnits().size() <<" detectors"<<std::endl;
	
	for(TrackerGeometry::DetUnitContainer::const_iterator it = pDD->detUnits().begin(); it != pDD->detUnits().end(); it++){
    
	   if( dynamic_cast<PixelGeomDetUnit*>((*it))!=0){
		DetId detid=(*it)->geographicalId();
			
		// fill bpix values for LA 
		if(detid.subdetId() == static_cast<int>(PixelSubdetector::PixelBarrel)) {
				
                PXBDetId pxdetid = PXBDetId(detid);
                cout << " hp:barrel:" << "  layer=" << pxdetid.layer() << "  ladder=" << pxdetid.ladder() << "  module=" << pxdetid.module() << endl;

		   if(!useFile_){

		        for(Parameters::iterator it = BPixParameters_.begin(); it != BPixParameters_.end(); ++it) {
                           if( it->getParameter<unsigned int>("module") == pxdetid.module() && it->getParameter<unsigned int>("layer") == pxdetid.layer() )
                           {
                              float something = (float)it->getParameter<double>("angle");
                              SomeThing->putSomeThing(detid.rawId(),something);
                           }
                        }

		   } else {
//			cout << "method for reading file not implemented yet" << endl;
  			edm::LogError("SiPixelSomeThingDB")<<"[SiPixelSomeThingDB::analyze] method for reading file not implemented yet" << std::endl;
		   }
			
		   // fill fpix values for LA 
		} else if(detid.subdetId() == static_cast<int>(PixelSubdetector::PixelEndcap)) {
				
                      PXFDetId pxdetid = PXFDetId(detid);
                      cout << " hp:endcap:" << "  side=" << pxdetid.side() << "  disk=" << pxdetid.disk() << "  blade=" << pxdetid.blade() << "  panel=" << pxdetid.panel() << "  module=" << pxdetid.module() << endl;

		        for(Parameters::iterator it = FPixParameters_.begin(); it != FPixParameters_.end(); ++it) {
                           if( it->getParameter<unsigned int>("side") == pxdetid.side() && it->getParameter<unsigned int>("disk") == pxdetid.disk() && it->getParameter<unsigned int>("HVgroup") == HVgroup(pxdetid.panel(),pxdetid.module()) )
                           {
                              float something = (float)it->getParameter<double>("angle");
                              SomeThing->putSomeThing(detid.rawId(),something);
                           }
                        }

		} else {
			edm::LogError("SiPixelSomeThingDB")<<"[SiPixelSomeThingDB::analyze] detid is Pixel but neither bpix nor fpix"<<std::endl;
		}
		
	   }
			
	}      
  	

	edm::Service<cond::service::PoolDBOutputService> mydbservice;
	if( mydbservice.isAvailable() ){
		try{
			if( mydbservice->isNewTagRequest(recordName_) ){
				mydbservice->createNewIOV<SiPixelSomeThing>(SomeThing,
									       mydbservice->beginOfTime(),
									       mydbservice->endOfTime(),
									       recordName_);
			} else {
				mydbservice->appendSinceTime<SiPixelSomeThing>(SomeThing,
										  mydbservice->currentTime(),
										  recordName_);
			}
		}catch(const cond::Exception& er){
			edm::LogError("SiPixelSomeThingDB")<<er.what()<<std::endl;
		}catch(const std::exception& er){
			edm::LogError("SiPixelSomeThingDB")<<"caught std::exception "<<er.what()<<std::endl;
		}catch(...){
			edm::LogError("SiPixelSomeThingDB")<<"Funny error"<<std::endl;
		}
	}else{
		edm::LogError("SiPixelSomeThingDB")<<"Service is unavailable"<<std::endl;
	}
   

}


unsigned int SiPixelSomeThingDB::HVgroup(unsigned int panel, unsigned int module){

   if( 1 == panel && ( 1 == module || 2 == module ))  {
      return 1;
   }
   else if( 1 == panel && ( 3 == module || 4 == module ))  {
      return 2;
   }
   else if( 2 == panel && 1 == module )  {
      return 1;
   }
   else if( 2 == panel && ( 2 == module || 3 == module ))  {
      return 2;
   }
   else {
      cout << " *** error *** in SiPixelSomeThingDB::HVgroup(...), panel = " << panel << ", module = " << module << endl;
      return 0;
   }
   
}


void SiPixelSomeThingDB::endJob(){


}


