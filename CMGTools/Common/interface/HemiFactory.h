#ifndef HEMIFACTORY_H_
#define HEMIFACTORY_H_

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "DataFormats/Common/interface/View.h"

#include "AnalysisDataFormats/CMGTools/interface/CompoundTypes.h"
#include "AnalysisDataFormats/CMGTools/interface/Hemisphere.h"
#include "CMGTools/Common/interface/Factory.h"

namespace cmg {

  class HemisphereFactory : public Factory<cmg::Hemisphere> {

  public:
    HemisphereFactory(const edm::ParameterSet& ps) :
      hemisphereLabel_(ps.getParameter<edm::InputTag>("inputCollection")),
      maxNCand_(20)
        {
        }
    typedef cmg::Factory<cmg::Hemisphere>::event_ptr event_ptr;
    virtual event_ptr create(const edm::Event&,
                             const edm::EventSetup&) const;

  private:

    edm::InputTag const hemisphereLabel_;

    size_t const maxNCand_;
  };

} // namespace cmg

#endif /*HEMIFACTORY_H_*/
