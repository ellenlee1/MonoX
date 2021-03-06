import os
import sys
import math
import array

thisdir = os.path.dirname(os.path.realpath(__file__))
basedir = os.path.dirname(thisdir)
sys.path.append(basedir)
from datasets import allsamples
import config
from main.plotconfig import getConfig
from plotstyle import SimpleCanvas

import ROOT
ROOT.gROOT.SetBatch(True)

def ColorGrad():
    ncontours = 999
    stops = (0.0,1.0)
    red   = (1.0,0.0)
    green = (1.0,0.0)
    blue  = (1.0,1.0)
    
    s = array.array('d', stops)
    r = array.array('d', red)
    g = array.array('d', green)
    b = array.array('d', blue)

    npoints = len(s)
    ROOT.TColor.CreateGradientColorTable(npoints, s, r, g, b, ncontours)
    ROOT.gStyle.SetNumberContours(ncontours)
    ROOT.gStyle.SetPaintTextFormat(".0f")

lumi = allsamples['sph-d3'].lumi + allsamples['sph-d4'].lumi
canvas =  SimpleCanvas(lumi = lumi, xmax = 0.90)

samples = [ 'monoph', 'efake', 'hfake', 'halo', 'haloUp', 'haloDown']

for sample in samples:
    dataTree = ROOT.TChain('events')
    dataTree.Add('/scratch5/ballen/hist/monophoton/skim/sph-d*_'+sample+'.root')

    xString = "t1Met.photonDPhi"
    yString = "( ( (photons.e55[0] / TMath::CosH(photons.eta[0])) - photons.pt[0] ) / t1Met.met )"

    dataHist = ROOT.TH2D(sample, "", 30, 0., math.pi, 10, -1.0, 1.0)
    dataHist.GetXaxis().SetTitle('#Delta#phi(#gamma, E_{T}^{miss})')
    dataHist.GetYaxis().SetTitle('(E_{55}^{#gamma} - E_{T}^{#gamma}) / E_{T}^{miss}')
    dataHist.Sumw2()
    dataTree.Draw(yString+":"+xString+">>"+sample, '(photons.pt[0] > 175. && t1Met.minJetDPhi > 0.5 && t1Met.met > 170.)', 'goff')

    ColorGrad()
    canvas.addHistogram(dataHist, drawOpt = 'COLZ TEXT')
    canvas.printWeb('monophoton/phoMet', 'DiffVsPhi'+'_'+sample, logy = False)

    canvas.Clear(xmax = 0.90)

dataTree = ROOT.TChain('events')
dataTree.Add('/scratch5/ballen/hist/monophoton/skim/sph-d*_monoph.root')

xString = "photons.eta[0]"
yString = "photons.phi[0]"

dataHist = ROOT.TH2D("lego", "", 30, -1.5, 1.5, 30, -math.pi, math.pi)
dataHist.GetXaxis().SetTitle('#eta')
dataHist.GetYaxis().SetTitle('#phi')
dataHist.Sumw2()
dataTree.Draw(yString+":"+xString+">>lego", 'photons.pt[0] > 175. && t1Met.minJetDPhi > 0.5 && t1Met.met > 170. && t1Met.photonDPhi < 2.', 'goff')

canvas.addHistogram(dataHist, drawOpt = 'COLZ TEXT')
canvas.printWeb('monophoton/phoMet', 'EtaPhiPlane', logy = False)

canvas.Clear(xmax = 0.90)

dataTree = ROOT.TChain('events')
dataTree.Add('/scratch5/ballen/hist/monophoton/skim/sph-d*_monoph.root')

xString = "photons.eta[0]"
yString = "photons.sieie[0]"

dataHist = ROOT.TH2D("shower", "", 30, -1.5, 1.5, 30, 0.006, 0.012)
dataHist.GetXaxis().SetTitle('#eta')
dataHist.GetYaxis().SetTitle('#sigma_{i#eta i#eta}')
dataHist.Sumw2()
dataTree.Draw(yString+":"+xString+">>shower", 'photons.pt[0] > 175. && t1Met.minJetDPhi > 0.5 && t1Met.met > 170. && t1Met.photonDPhi < 2.', 'goff')

canvas.addHistogram(dataHist, drawOpt = 'COLZ TEXT')
canvas.printWeb('monophoton/phoMet', 'EtaSieiePlane', logy = False)

canvas.Clear(xmax = 0.90)

dataTree = ROOT.TChain('events')
dataTree.Add('/scratch5/ballen/hist/monophoton/skim/sph-d*_gjets.root')

xString = "photons.chWorstIso[0]"
yString = "photons.sieie[0]"

dataHist = ROOT.TH2D("fakes", "", 20, 0., 10., 24, 0.000, 0.024)
dataHist.GetXaxis().SetTitle('CH Worst Iso (GeV)')
dataHist.GetYaxis().SetTitle('#sigma_{i#eta i#eta}')
dataHist.Sumw2()
dataTree.Draw(yString+":"+xString+">>fakes", 'photons.pt[0] > 175. && t1Met.minJetDPhi > 0.5 && t1Met.met > 170. && t1Met.photonDPhi < 0.5', 'goff')

canvas.addHistogram(dataHist, drawOpt = 'COLZ TEXT')
canvas.printWeb('monophoton/phoMet', 'IsoSieiePlane', logy = False)

canvas.Clear(xmax = 0.90)

dataTree = ROOT.TChain('events')
dataTree.Add('/scratch5/ballen/hist/monophoton/skim/sph-d*_monoph.root')

xString = "( run / 1000.)"
yString = "lumi"

dataHist = ROOT.TH2D("runlumi", "", 9, 256.5, 261.0, 10, 0., 200.)
dataHist.GetXaxis().SetTitle('Run Number / 1000.')
dataHist.GetYaxis().SetTitle('Lumi Section')
dataHist.Sumw2()
dataTree.Draw(yString+":"+xString+">>runlumi", 'photons.pt[0] > 175. && t1Met.minJetDPhi > 0.5 && t1Met.met > 170. && t1Met.photonDPhi < 0.5', 'goff')

canvas.addHistogram(dataHist, drawOpt = 'COLZ TEXT')
# canvas.logx = True
canvas.printWeb('monophoton/phoMet', 'RunLumiPlane', logy = False)

canvas.Clear(xmax = 0.90)

dataTree = ROOT.TChain('events')
dataTree.Add('/scratch5/ballen/hist/monophoton/skim/sph-d*_monoph.root')

yString = "t1Met.photonDPhi"
xString = "std::abs(TVector2::Phi_mpi_pi(jets.phi[0] - t1Met.phi))"

dataHist = ROOT.TH2D("dphisquared", "", 30, 0., math.pi, 30, 0., math.pi)
dataHist.GetYaxis().SetTitle('#Delta#phi(#gamma, E_{T}^{miss})')
dataHist.GetXaxis().SetTitle('#Delta#phi(leading jet, E_{T}^{miss})')
dataHist.Sumw2()
dataTree.Draw(yString+":"+xString+">>dphisquared", 'photons.pt[0] > 175. && t1Met.minJetDPhi > 0.5 && t1Met.met > 170.', 'goff')

canvas.addHistogram(dataHist, drawOpt = 'COLZ TEXT')
# canvas.logx = True
canvas.printWeb('monophoton/phoMet', 'DoubleDPhiPlane', logy = False)

canvas.Clear(xmax = 0.90)

dataTree = ROOT.TChain('events')
dataTree.Add('/scratch5/ballen/hist/monophoton/skim/sph-d*_monoph.root')

xString = "jets.eta"
yString = "jets.phi"

dataHist = ROOT.TH2D("jetlego", "", 50, -5., 5., 30, -math.pi, math.pi)
dataHist.GetXaxis().SetTitle('#eta_{jet}')
dataHist.GetYaxis().SetTitle('#phi_{jet}')
dataHist.Sumw2()
dataTree.Draw(yString+":"+xString+">>jetlego", 'photons.pt[0] > 175. && t1Met.minJetDPhi > 0.5 && t1Met.met > 170. && t1Met.photonDPhi < 0.5', 'goff')

canvas.addHistogram(dataHist, drawOpt = 'COLZ TEXT')
canvas.printWeb('monophoton/phoMet', 'JetEtaPhiPlane', logy = False)
