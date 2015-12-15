#ifndef _AODReader_h_
#define _AODReader_h_
#include "AthenaBaseComps/AthAlgorithm.h"

#include "GaudiKernel/NTuple.h"
#include "GaudiKernel/ITHistSvc.h"

#include "xAODTruth/TruthParticleContainer.h"
#include "TH1F.h"

#include <string>
#include <map>

class TTree;
/*namespace xAOD {
	class TruthParticleContainer;
}
*/
class AODReader : public AthAlgorithm {

 public:

   AODReader (const std::string& name, ISvcLocator* pSvcLocator);
   virtual ~AODReader ();

   virtual StatusCode initialize();
   virtual StatusCode execute();
   virtual StatusCode finalize();

 private:

//   NTuple::Tuple *m_nt;
   float m_energy;
   float m_eta;
   float m_pt;
   float m_e237;
   float m_e277;
   float m_weta1;
   float m_weta2;
   float m_e2tsts1;
   float m_fracs1;
   float m_wtots1;
   float m_f1;
   float m_f1core;
   float m_et;
   float m_ethad1;
   float m_emins1;

   int m_truth_energy;
   float m_truth_px;
   float m_truth_py;
   float m_truth_pz;
   bool m_frozenShower;
   bool m_makeHist;
   std::string m_histName = "energy";
   std::string m_histTitle = "Reconstructed electron energy";
   typedef std::map<int, TH1F*> energyHistMap;
   energyHistMap histMap;
   TTree* m_ntuple;
   ServiceHandle<ITHistSvc> m_ntsvc;
   const xAOD::TruthParticleContainer *mcEvtColl;
};

#endif
