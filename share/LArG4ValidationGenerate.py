#!/usr/bin/env physh
# print command line for logging
from optparse import OptionParser
parser = OptionParser(usage = "usage: %prog", version="%prog $Id: LArG4ValidationGenerate.py 583537 2014-02-15 21:17:29Z zmarshal $")

#parser.add_option("--cpuNumber", dest="cpu",         default='00',                       help="select the desired CPU")
parser.add_option("--inputFile",  dest="input",       default=None,                       help="select the input benevent file name (preferred over the particle gun)")
parser.add_option("--outputFile", dest="output",      default='sp',                       help="select the output file name prefix (default: %default)")
parser.add_option("--geometryVersion",   dest="geometry",    default='',                         help="select the geometry tag (leave empty for auto)")
parser.add_option("--conditionsTag",  dest="condition",   default='',                         help="select the condition tag (REQUIRED)")
parser.add_option("--physicsLis",   dest="physlist",    default='',                         help="select the physics list (leave empty for auto)")
parser.add_option("--runNumber",  dest="runNumber",   default=0,              type=int,   help="run number (default: %default)")
parser.add_option("--etaMin",     dest="etaMin",      default='-5.5',                     help="particle gun: Minimum Eta (default: %default)")
parser.add_option("--etaMax",     dest="etaMax",      default='5.5',                      help="particle gun: Maximum Eta (default: %default)")
parser.add_option("--energyMin",  dest="energyMin",   default='20000',                    help="particle gun: particle Minimum energy (default: %default MeV)")
parser.add_option("--energyMax",  dest="energyMax",   default='500000',                   help="particle gun: particle Maximum energy (default: %default MeV)")
parser.add_option("--pid",        dest="pid",         default='11',                       help="particle gun: particle type (11=electron, 13=muon, 111=pi0 211=pi+, .., default: %default)")
parser.add_option("--parameterize",dest="parameterize", default=0,            type=int,   help="parameteriziation level (0=off, default: %default)")
parser.add_option("--nevents",    dest="nevents",     default=-1,             type=int,   help="number of events to generate (default: all)")
parser.add_option("--configFile", dest="configFileName", default='',   help="name of file with configuration (default: none)");
parser.add_option("--FSLib",      dest="fsLibs",      default=[],      action="append",   help="path to the frozen shower libraries (separate flag for each library must be used)")
(options, args) = parser.parse_args()


from AthenaCommon.DetFlags import DetFlags
# - Select detectors 
DetFlags.ID_setOn()
DetFlags.Calo_setOn()
DetFlags.Muon_setOn()
DetFlags.Truth_setOn()


# get a handle to the default top-level algorithm sequence
from AthenaCommon.AlgSequence import AlgSequence
from AthenaCommon.AppMgr import ServiceMgr
topSequence = AlgSequence()

## Output threshold (DEBUG, INFO, WARNING, ERROR, FATAL)
ServiceMgr.MessageSvc.OutputLevel = INFO

from AthenaCommon.GlobalFlags import globalflags
if len(options.condition) > 0 :
    globalflags.ConditionsTag = options.condition
else :
    print "No condition tag specified.\nThere is no use in validating FS with some arbitrary condition tag.\n Please, specify one."
    import sys
    sys.exit(1)


from AthenaCommon.AthenaCommonFlags import athenaCommonFlags

athenaCommonFlags.PoolHitsOutput=options.output + '.HITS.pool.root'

athenaCommonFlags.EvtMax=options.nevents


#--- Simulation flags -----------------------------------------
from G4AtlasApps.SimFlags import SimFlags
SimFlags.load_atlas_flags()
SimFlags.CalibrationRun.set_Off()

# Random seeds get random values
from random import randint
SimFlags.RandomSeedOffset = randint(1,1000000000)

if len(options.geometry) > 0 :
    SimFlags.SimLayout = options.geometry
else :
    SimFlags.SimLayout.set_On()

if len(options.physlist) > 0 :
    SimFlags.PhysicsList = options.physlist

if options.input is not None:
    athenaCommonFlags.PoolEvgenInput=[options.input]
else:
        ## Use single particle gun
        import AthenaCommon.AtlasUnixGeneratorJob
        import ParticleGun as PG
        topSequence += ParticleGun()
        topSequence.ParticleGun.sampler.pid = (pid)
        if options.configFileName is not "" :
            energy={}
            inputfile=open(options.ConfigFileName);
            for line in inputfile:
                for v in line.split():
                    data.append(int(v))
            energyBins=set(data);   
            topSequence.ParticleGun.sampler.mom = PG.EEtaMPhiSampler(energy=energyBins, eta=[3.8, 4.8])
        else:
            topSequence.ParticleGun.sampler.mom = PG.EEtaMPhiSampler(energy=[energyMin, energyMax], eta=[3.8, 4.8])
        evgenConfig.generators += ["ParticleGun"]
        athenaCommonFlags.PoolEvgenInput.set_Off()


if (options.parameterize > 0): 
    SimFlags.LArParameterization=options.parameterize

    if len(options.fsLibs) > 0 :
        print "Setting up ShowerLib Service"
        from LArG4ShowerLibSvc.LArG4ShowerLibSvcConf import LArG4ShowerLibSvc
        if not hasattr( ServiceMgr, 'LArG4ShowerLibSvc' ):
             ServiceMgr += LArG4ShowerLibSvc()
        ServiceMgr.LArG4ShowerLibSvc.FileNameList = options.fsLibs
    
    
## Set Event #
SimFlags.RunNumber = options.runNumber

from G4AtlasApps.PyG4Atlas import PyG4AtlasAlg
topSequence += PyG4AtlasAlg()

from LArG4Validation.LArG4ValidationConf import SingleTrackValidation
topSequence += SingleTrackValidation()

# The following lines are to construct our ntuple:
RootFileName = options.output + '.tuple.root'

NtupleSvc = Service( "NTupleSvc" )
NtupleSvc.Output = [ "FILE DATAFILE=\'" + RootFileName + "\' OPT='NEW'" ]

theApp.Dlls   +=  [  "RootHistCnv"   ]
theApp.HistogramPersistency = "ROOT"
