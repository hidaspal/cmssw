#ifndef SiPixelSomeThing_h
#define SiPixelSomeThing_h

#include<vector>
#include<map>
#include<iostream>
#include<boost/cstdint.hpp>


class SiPixelSomeThing {

 public:
 
  SiPixelSomeThing(){};
  ~SiPixelSomeThing(){};

  inline void putSomeThings(std::map<unsigned int,float>& LA){m_LA=LA;}   
  inline const std::map<unsigned int,float>&  getSomeThings () const {return m_LA;}

  bool   putSomeThing(const uint32_t&, float&);
  float  getSomeThing (const uint32_t&) const;


 private:
  std::map<unsigned int,float> m_LA; 
};

#endif
