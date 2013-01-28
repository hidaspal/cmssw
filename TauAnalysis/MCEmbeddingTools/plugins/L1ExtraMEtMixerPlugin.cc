#include "TauAnalysis/MCEmbeddingTools/plugins/L1ExtraMEtMixerPlugin.h"

#include "FWCore/Utilities/interface/Exception.h"

#include "DataFormats/L1Trigger/interface/L1EtMissParticle.h"
#include "DataFormats/L1Trigger/interface/L1EtMissParticleFwd.h"
#include "DataFormats/Common/interface/Handle.h"

#include <TMath.h>

#include <vector>
#include <set>
#include <algorithm>

L1ExtraMEtMixerPlugin::L1ExtraMEtMixerPlugin(const edm::ParameterSet& cfg)
  : L1ExtraMixerPluginBase(cfg),
    srcMuons_(cfg.getParameter<edm::InputTag>("srcMuons"))
{}

void L1ExtraMEtMixerPlugin::registerProducts(edm::EDProducer& producer)
{
  producer.produces<l1extra::L1EtMissParticleCollection>(instanceLabel_); 
}

void L1ExtraMEtMixerPlugin::produce(edm::Event& evt, const edm::EventSetup& es)
{
  //std::cout << "<L1ExtraMEtMixerPlugin::produce>:" << std::endl;
  //std::cout << " src1 = " << src1_ << std::endl;
  //std::cout << " src2 = " << src2_ << std::endl;
  //std::cout << " instanceLabel = " << instanceLabel_ << std::endl;

  edm::Handle<l1extra::L1EtMissParticleCollection> met1;
  evt.getByLabel(src1_, met1);
  
  edm::Handle<l1extra::L1EtMissParticleCollection> met2;
  evt.getByLabel(src2_, met2);

  edm::Handle<reco::CandidateCollection> muons;
  evt.getByLabel(srcMuons_, muons);

  // CV: keep code general and do not assume 
  //     that there is exactly one MET object per event.
  //     The number of objects stored in the 'src1' and 'src2' do need to match, however,
  //     in order for the MET to be added vectorially
  if ( met1->size() != met2->size() )
    throw cms::Exception("L1ExtraMEtMixer::produce")
      << " Mismatch in numbers of MET objects stored in collections 'src1' and 'src2' !!\n";
  
  std::auto_ptr<l1extra::L1EtMissParticleCollection> metSum(new l1extra::L1EtMissParticleCollection());

  // CV: entries in MET collections refer to different bunch-crossings.
  //     The number of entries in the two MET collections is not necessarily the same
  //    --> match entries by bunch-crossing number
  std::set<int> bx_set;
  for ( l1extra::L1EtMissParticleCollection::const_iterator met1_i = met1->begin();
	met1_i != met1->end(); ++met1_i ) {
    bx_set.insert(met1_i->bx());
  }
  for ( l1extra::L1EtMissParticleCollection::const_iterator met2_i = met2->begin();
	met2_i != met2->end(); ++met2_i ) {
    bx_set.insert(met2_i->bx());
  }

  std::vector<int> bx_vector;
  for ( std::set<int>::const_iterator bx = bx_set.begin();
	bx != bx_set.end(); ++bx ) {
    bx_vector.push_back(*bx);
  }
  std::sort(bx_vector.begin(), bx_vector.end());

  for ( std::vector<int>::const_iterator bx = bx_vector.begin();
	bx != bx_vector.end(); ++bx ) {
    bool errorFlag = false;

    const l1extra::L1EtMissParticle* met1_bx = 0;
    for ( l1extra::L1EtMissParticleCollection::const_iterator met1_i = met1->begin();
	  met1_i != met1->end(); ++met1_i ) {
      if ( met1_i->bx() == (*bx) ) {
	if ( met1_bx ) errorFlag = true;
	met1_bx = &(*met1_i);
      }
    }

    const l1extra::L1EtMissParticle* met2_bx = 0;
    for ( l1extra::L1EtMissParticleCollection::const_iterator met2_i = met2->begin();
	  met2_i != met2->end(); ++met2_i ) {
      if ( met2_i->bx() == (*bx) ) {
	if ( met2_bx ) errorFlag = true;
	met2_bx = &(*met2_i);
      }
    }

    if ( errorFlag )
      throw cms::Exception("L1ExtraMEtMixer::produce")
	<< " Failed to find unique match of MET objects for BX = " << (*bx) << " !!\n";
    assert(met1_bx || met2_bx);
    
     // CV: check that both MET objects are of the same type
    if ( met1_bx && met2_bx && met1_bx->type() != met2_bx->type() )
      throw cms::Exception("L1ExtraMEtMixer::produce")
	<< " Mismatch in type between MET objects stored in collections 'src1' and 'src2' !!\n";

    double metSumPx = 0.;
    double metSumPy = 0.;
    double metSumEt = 0.;
    int type = -1;
    if ( met1_bx ) {
      //std::cout << "met1 (BX = " << (*bx) << "): Px = " << met1_bx->px() << ", Py = " << met1_bx->py() 
      //	  << " (Et = " << met1_bx->etMiss() << ", Pt = " << met1_bx->pt() << ", phi = " << met1_bx->phi() << ")" << std::endl;
      metSumPx += met1_bx->px();
      metSumPy += met1_bx->py();
      metSumEt += met1_bx->etTotal();
      type = met1_bx->type();
    }
    if ( met2_bx ) {
      //std::cout << "met2 (BX = " << (*bx) << "): Px = " << met2_bx->px() << ", Py = " << met2_bx->py() 
      //	  << " (Et = " << met2_bx->etMiss() << ", Pt = " << met2_bx->pt() << ", phi = " << met2_bx->phi() << ")" << std::endl;
      metSumPx += met2_bx->px();
      metSumPy += met2_bx->py();
      metSumEt += met2_bx->etTotal();
      type = met2_bx->type();
    }
    double metSumPt = TMath::Sqrt(metSumPx*metSumPx + metSumPy*metSumPy);

    // Subtract contribution by muons that were removed from the event
    for(reco::CandidateCollection::const_iterator iter = muons->begin(); iter != muons->end(); ++iter)
    {
      const reco::Candidate& cand = *iter;

      // We subtract the average L1 ETM reconstructed for a single muon, which was determined
      // in independent studies (see AnaL1CaloCleaner.cc).
      double X = 0.0;
      if(abs(cand.eta()) < 1.2)
        X = 1.941 / cand.pt();
      else if(abs(cand.eta()) < 1.7)
        X = 1.261 / cand.pt();
      else
        X = 0.5 / cand.pt();

      // The direction is given by the muon direction
      metSumPx += X * cand.px();
      metSumPy += X * cand.py();
      metSumEt += X * cand.et();
    }

    // CV: setting edm::Refs to L1Gct objects not implemented yet
    l1extra::L1EtMissParticle metSum_bx(
      reco::Candidate::LorentzVector(metSumPx, metSumPy, 0., metSumPt),
      (l1extra::L1EtMissParticle::EtMissType)type,
      metSumEt,
      edm::Ref<L1GctEtMissCollection>(),
      edm::Ref<L1GctEtTotalCollection>(),
      edm::Ref<L1GctHtMissCollection>(),
      edm::Ref<L1GctEtHadCollection>(),
      (*bx));
    //std::cout << "metSum (BX = " << (*bx) << "): Px = " << metSum_bx.px() << ", Py = " << metSum_bx.py() 
    //	        << " (Et = " << metSum_bx.etMiss() << ", Pt = " << metSum_bx.pt() << ", phi = " << metSum_bx.phi() << ")" << std::endl;
    metSum->push_back(metSum_bx);					       
  }
  
  evt.put(metSum, instanceLabel_);
}

#include "FWCore/Framework/interface/MakerMacros.h"

DEFINE_EDM_PLUGIN(L1ExtraMixerPluginFactory, L1ExtraMEtMixerPlugin, "L1ExtraMEtMixerPlugin");
