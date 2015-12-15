import ROOT
from ROOT import *

usage = "usage: %prog [options] root1 [root2 ...]"

parser = OptionParser(usage=usage, version="%prog v1.0 $Id: LArG4ValidationPlotter.py 505706 2012-06-15 11:48:02Z gsedov $")

parser.add_option("-o","--output",dest="outputfile", help="output ps file")
#parser.add_option("-d","--divide",dest="divide",action="store_true", help="Calculate ratio between root files (divide all roots to first")
#parser.add_option("-m","--mean",dest="mean",action="store_true",help="Print mean value in the legend")
parser.set_defaults(outputfile="output.root")

(options, args) = parser.parse_args()


numberOfSigmaToFit=4;
minimumEvntPerBin=7

def makeHistLookPretty (hist):
	xAxis=hist.GetXaxis();
	#finding better range for histogram
	rms = hist.GetRMS()
	interval = rms*numberOfSigmaToFit;
	mean = hist.GetMean();

	#finding better binning for histogram
	binWidth=xAxis.GetBinWidth(1)
	#number of bins within interval of interest
	numberOfBins=numberOfSigmaToFit*hist.GetRMS()/binWidth;
	evntPerBin=hist.Integral()/numberOfBins;
	while evntPerBin<=minimumEvntPerBin:
		hist.Rebin(2);
		evntPerBin=evntPerBin*2;

	fitFunction = TF1("fitFunction", "gaus", mean-interval, mean+interval)
	fitFunction.SetParNames("constant", "mean", "sigma")

	##setting primary point for fitter
	fitFunction.SetParameter("mean", mean)
	fitFunction.SetParameter("sigma", rms)

	hist.Fit("fitFunction", "WWR")	
	xAxis.SetRangeUser(mean-interval,mean+interval);

	return fitFunction

totalMean = []
totalSigma = []
legendSigma=TLegend()
legendMean=TLegend()

graphsCount = 0

fileOut = ROOT.TFile.Open(options.outputfile, "RECREATE")

for fileIn in args:
	fileHist = ROOT.TFile.Open(fileIn)
	nameSim = fileIn.replace(".root", "")
	graphSigma = TGraphErrors()
	graphMean  = TGraphErrors()
	for folder in fileHist.GetListOfKeys():
		truthEnergy = int(folder.GetName())
		energyDir = fileHist.Get(folder.GetName())
		for key in energyDir.GetListOfKeys():
			hist=key.ReadObj().Clone()
			fitFunction = makeHistLookPretty(hist);
			graphSigma.SetPoint(graphsCount, truthEnergy, fitFunction.GetParameter("sigma")/truthEnergy)
			graphSigma.SetPointError(graphsCount, 0, fitFunction.GetParError("sigma")/truthEnergy)
			graphMean.SetPoint(graphsCount, truthEnergy, fitFunction.GetParameter("mean")-truthEnergy)
			graphMean.SetPointError(graphsCount, 0, fitFunction.GetParError("mean"))
	legendSigma.AddEntry(graphSigma, nameSim)
	legendMean.AddEntry(graphMean, nameSim)
	totalMean.append(graphSigma)
	totalSigma.append(graphMean)

canvMean= TCanvas()
canvSigma = TCanvas()
for sigma,mean in zip (totalSigma,totalMean):
		canvSigma.cd()
		sigmaHist=sigma.GetHistogram()
		sigmaHist.Draw("same")
		canvMean.cd()
		meanHist = mean.GetHistogram()
		meanHist.Draw("same")
canvMean.cd()
legendMean.Draw()
canvMean.Write()
canvSigma.cd()
legendSigma.Draw()
canvSigma.Write()
fileOut.Write()
fileOut.Close()




