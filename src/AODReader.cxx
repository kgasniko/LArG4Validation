#include <math.h> 
#include "LArG4Validation/AODReader.h"

#include "GaudiKernel/IToolSvc.h"
#include "egammaEvent/Electron.h"
#include "egammaEvent/ElectronContainer.h"
#include "egammaEvent/EMShower.h"
//
#include "GeneratorObjects/McEventCollection.h"

#include "MCTruthClassifier/MCTruthClassifier.h"
#include "xAODEgamma/Egamma.h"
#include "xAODEgamma/EgammaContainer.h"
#include "xAODEgamma/EgammaxAODHelpers.h"
#include "xAODEgamma/ElectronxAODHelpers.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODCaloEvent/CaloClusterContainer.h"
#include "xAODTracking/TrackParticle.h"
#include <string.h>
#include "TTree.h"
#include <iostream>
#include "StoreGate/StoreGateSvc.h"
#include "GaudiKernel/NTuple.h"
#include "GaudiKernel/INTupleSvc.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/Property.h"
//#include "xAODTruth/xAODTruthHelper.h"
//

const int authorElectronOrForward=1+8;

AODReader::AODReader (const std::string& name, ISvcLocator* pSvcLocator) 
  : AthAlgorithm(name, pSvcLocator),
    m_ntsvc("THistSvc/THistSvc", name)
{
	declareProperty("makeHist", m_makeHist=true, "key to make output file with histograms");
}

AODReader::~AODReader () { }

StatusCode AODReader::initialize()
{
  msg(MSG::INFO) << "Initializing " << name() << endmsg;

  msg(MSG::INFO) << "creating ntuple" << endmsg;

  if (!m_ntsvc.retrieve().isSuccess()) {
	   ATH_MSG_ERROR("could not retrieve handle to ntuple svc");
	   return StatusCode::FAILURE;
  }

  if (!m_makeHist) { //create ntuples if requested

  	  m_ntuple = new TTree("parameters", "Tree with basic electron parameters");

  	  if (!m_ntsvc->regTree("/rec/trees/parameters", m_ntuple).isSuccess()) {
	  	ATH_MSG_ERROR("could not register tree [parameters]");
	   	delete (m_ntuple); 
		m_ntuple = 0;
	   	return StatusCode::FAILURE;
  	  }

	  //booking branches
	  m_ntuple->Branch("eta"		, &this->m_eta, 		"eta/f");
	  m_ntuple->Branch("energy"		, &this->m_energy, 	"energy/f");
	  m_ntuple->Branch("pt"			, &this->m_pt, 		"pt/f");
	  m_ntuple->Branch("e237"		, &this->m_e237, 	"e237/f");
	  m_ntuple->Branch("e277"		, &this->m_e277, 	"e277/f");
	  m_ntuple->Branch("weta1"		, &this->m_weta1, 	"weta1/f");
	  m_ntuple->Branch("weta2"		, &this->m_weta2, 	"weta2/f");
	  m_ntuple->Branch("e2tsts1"	, &this->m_e2tsts1, 	"e2tsts1/f");
	  m_ntuple->Branch("emins1"		, &this->m_emins1, 	"emins1/f");
	  m_ntuple->Branch("fracs1"		, &this->m_fracs1, 	"fracs1/f");
	  m_ntuple->Branch("wtots1"		, &this->m_wtots1, 	"wtots1/f");
	  m_ntuple->Branch("f1"			, &this->m_f1, 		"f1/f");
	  m_ntuple->Branch("f1core"		, &this->m_f1core, 	"f1core/f");
	  m_ntuple->Branch("ethad1"		, &this->m_ethad1, 	"ethad1/f");
	  m_ntuple->Branch("et"			, &this->m_et, 		"et/f");
	  //truth information branches 
	  m_ntuple->Branch("truth_px"	, &this->m_truth_px, "truth_px/f");
	  m_ntuple->Branch("truth_py"	, &this->m_truth_py, "truth_py/f");
	  m_ntuple->Branch("truth_pz"	, &this->m_truth_pz, "truth_pz/f");
	}
  //FIXME!
  return StatusCode::SUCCESS;
}

StatusCode AODReader::finalize()
{
  msg(MSG::INFO) << "Finalised " << name() << endmsg;
  return StatusCode::SUCCESS;
}

StatusCode AODReader::execute()
{

  StatusCode sc = StatusCode::SUCCESS;

  sc=evtStore()->retrieve(this->mcEvtColl, "TruthParticles");
  if ( sc.isFailure() || !mcEvtColl ) {
	  msg(MSG::WARNING) << "Could not retrieve TruthParticleContainer" << endmsg;
	  return sc;
  }

  msg(MSG::INFO) << "TruthParticleContainer successfully retrieved" << endmsg;

  
  /// read the xAOD electron containers
  const xAOD::ElectronContainer* electronContainer; 
  sc=evtStore()->retrieve( electronContainer, "Electrons");

  if( sc.isFailure()  ||  !electronContainer ) {
    msg(MSG::WARNING) << "No AOD electron container found in TDS" << endmsg;
    return sc;
  }


  const xAOD::ElectronContainer* forwardContainer;
  sc=evtStore()->retrieve( forwardContainer, "ForwardElectrons");
  if( sc.isFailure()  ||  !forwardContainer ) {
    msg(MSG::WARNING) << "No xAOD forward electron container found in TDS" << endmsg;
    return sc;
  }

  msg(MSG::INFO) << "ElectronContainer successfully retrieved" << endmsg;
  

  //read the xAOD thruth particle container
    float val_float = 0;
  float val_inval = -1000.;
  int size = 0;
  int maxTruthElecEnergy = 0;
  for (auto particle: *mcEvtColl){
	  if (particle->nParents() != 0 || !particle->isElectron())
		  continue;
	  m_truth_energy = int(round(particle->e()/1000));
	  if (maxTruthElecEnergy < m_truth_energy)
		  maxTruthElecEnergy=m_truth_energy;
	  if (!m_makeHist){
			m_truth_px = particle->px();
			m_truth_py = particle->py();
			m_truth_pz = particle->pz();
	  }

  } 

  float elecEnergyMax=0;

  for (auto electron: *forwardContainer) {
	  if ((electron->author()  & authorElectronOrForward) == 0) //checking that this definetelly electron
			continue;

	  m_energy = electron->e();
	  if (m_energy>elecEnergyMax) //checking for case of several electrons
		  	elecEnergyMax = m_energy;
	  
	  //don't want just histograms? We will make trees! 
	  if (!m_makeHist){

			m_eta	= 	electron->p4().Eta();
			m_pt  	= 	electron->p4().Pt();

			//fillin shower shape information
			m_e237 	= 	electron->showerShapeValue(val_float,xAOD::EgammaParameters::e237) ? val_float : val_inval;
			m_e277 	= 	electron->showerShapeValue(val_float,xAOD::EgammaParameters::e277) ? val_float : val_inval;
			m_weta1 = 	electron->showerShapeValue(val_float,xAOD::EgammaParameters::weta1) ? val_float : val_inval;
			m_weta2 = 	electron->showerShapeValue(val_float,xAOD::EgammaParameters::weta2) ? val_float : val_inval;
			m_e2tsts1 = electron->showerShapeValue(val_float,xAOD::EgammaParameters::e2tsts1) ? val_float : val_inval;
			m_emins1 = 	electron->showerShapeValue(val_float,xAOD::EgammaParameters::emins1) ? val_float : val_inval;
			m_fracs1 = 	electron->showerShapeValue(val_float,xAOD::EgammaParameters::fracs1) ? val_float : val_inval;
			m_wtots1 = 	electron->showerShapeValue(val_float,xAOD::EgammaParameters::wtots1) ? val_float : val_inval;
			m_f1 	 = 	electron->showerShapeValue(val_float,xAOD::EgammaParameters::f1) ? val_float : val_inval;
			m_f1core =	electron->showerShapeValue(val_float,xAOD::EgammaParameters::f1core) ? val_float : val_inval;
			m_ethad1 = 	electron->showerShapeValue(val_float,xAOD::EgammaParameters::ethad1) ? val_float : val_inval;

			//filling cluster information
    	    const xAOD::CaloCluster*  caloCluster           = electron->caloCluster();
			if (caloCluster) { // don't know if it's possible for this not to exist, but just in case
				m_et = caloCluster->e()/TMath::CosH( fabs(caloCluster->eta()));
			} else {
				m_et = 0;
			}
			// writing a record
   		    if (m_ntuple->Fill() == -1) {
			     ATH_MSG_WARNING("problems writing n-tuple");
			}
	  }

  }

  if (m_makeHist) {	
	  energyHistMap::iterator mapIterator = histMap.find(m_truth_energy);

	if (mapIterator == histMap.end() || histMap.empty()){
		TH1F* tempHist = new TH1F(m_histName.c_str(), m_histTitle.c_str(), 100, maxTruthElecEnergy*0.3*1000, maxTruthElecEnergy*1.7*1000);
		tempHist->Fill(elecEnergyMax);
		sc=m_ntsvc->regHist("/stat/"+std::to_string(maxTruthElecEnergy)+"/"+m_histName, tempHist);
		if (sc.isFailure()) {
		      msg(MSG::WARNING) << "Could not register histogram " << std::to_string(maxTruthElecEnergy) << " / " << m_histName << endmsg;
		      return sc;
			}
		histMap.emplace(maxTruthElecEnergy, tempHist);
	}else {
		mapIterator->second->Fill(elecEnergyMax);
	}
  }

  return StatusCode::SUCCESS;
}
