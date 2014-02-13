#include "CondFormats/SiPixelObjects/interface/SiPixelSomeThing.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

bool SiPixelSomeThing::putSomeThing(const uint32_t& detid, float& value){
  std::map<unsigned int,float>::const_iterator id=m_LA.find(detid);
  if(id!=m_LA.end()){
    edm::LogError("SiPixelSomeThing") << "SiPixelSomeThing for DetID " << detid << " is already stored. Skippig this put" << std::endl;
    return false;
  }
  else m_LA[detid]=value;
  return true;
}
float SiPixelSomeThing::getSomeThing(const uint32_t& detid) const  {
  std::map<unsigned int,float>::const_iterator id=m_LA.find(detid);
  if(id!=m_LA.end()) return id->second;
  else {
    edm::LogError("SiPixelSomeThing") << "SiPixelSomeThing for DetID " << detid << " is not stored" << std::endl; 
  }
  return 0;
}
