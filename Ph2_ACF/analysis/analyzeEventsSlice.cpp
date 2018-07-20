#include <TTree.h>
#include <TH1D.h>
#include <TF1.h>
#include <TCanvas.h>
#include <TGraphErrors.h>
#include <TStyle.h>

#include <iostream>

TTree* myTree;
TH1D* myHistoIntegral[9];
TH1D* myHistoDifferential[9];
TGraphErrors* myGraphDifferential[9];

Int_t TDC;
Int_t nHits;
Int_t thresh;

bool initialized = false;

void addPoint(TGraphErrors* aGraph, double xx, double yy, double xx_error = -1, double yy_error = -1) {
  int iPoint = aGraph->GetN();
  std::cout << Form("point %d x=%f, y=%f\n", iPoint, xx, yy);
  aGraph->SetPoint(iPoint, xx, yy);
  if ((xx_error>0)||(yy_error>0)) {
    aGraph->SetPointError(iPoint, xx_error, yy_error);
  }
}

void initialize() {
  if (initialized) return;
  
  myTree = new TTree();
  myTree->ReadFile("tempfile.dat");

  myTree->SetBranchAddress("TDC",&TDC);
  myTree->SetBranchAddress("nHits",&nHits);
  myTree->SetBranchAddress("thresh",&thresh);
  
  for (int i=0; i<9; ++i) {
    myHistoIntegral[i] = new TH1D(Form("histoIntegral%d", i), Form("Integral distribution TDC=%d", i+4), 35, 0, 280);
    myHistoDifferential[i] = new TH1D(Form("histoDifferential%d", i), Form("Differential distribution TDC=%d", i+4), 35, 0, 280);
    myGraphDifferential[i] = new TGraphErrors();
    myGraphDifferential[i]->SetTitle(Form("Differential distribution TDC=%d", i+4));
  }

  initialized = true;
}

void analyzeEventsSlice(double onDelta = 100.0) {
  gStyle->SetOptFit(1);
  initialize();

  int decodedTDC;
  for (Long64_t iEvent=0; iEvent<myTree->GetEntries();iEvent++) {
    std::cout << "Processing Event " << iEvent << std::endl;
    myTree->GetEntry(iEvent);
    decodedTDC = TDC-4;
    if (decodedTDC<0) continue;
    if (decodedTDC>8) {
      std::cerr << "Help, this should not happen" << std::endl;
      continue;
    }
    if (nHits>0) myHistoIntegral[decodedTDC]->Fill(thresh, 1 /* nHits*/ );
    //myHistoIntegral[decodedTDC]->Fill(thresh, nHits );
  }


  TH1D* antani = new TH1D("diff", Form("diffTDC_%f", onDelta), 10, -0.5, 9.5);
  bool gotTDC[13];
  for (int i=0; i<13; ++i) { gotTDC[i]=false; }
  for (int iTDC=0; iTDC<9; ++iTDC) {
    double thisVal, prevVal;
    double thisX, prevX;
    prevVal=0;
    prevX=-9999;
    bool firstVal = true;
    for (int iBin=1; iBin<myHistoIntegral[iTDC]->GetNbinsX(); ++iBin) {
      thisVal = myHistoIntegral[iTDC]->GetBinContent(iBin);
      thisX = myHistoIntegral[iTDC]->GetXaxis()->GetBinCenter(iBin);
      if (thisVal==0) continue;

      if (!firstVal) {
	addPoint(myGraphDifferential[iTDC],
		 (thisX+prevX)/2., prevVal-thisVal
		 , fabs(thisX-prevX)/2., sqrt(prevVal+thisVal));	
        if (((thisX+prevX)/2.>onDelta)&&(!gotTDC[iTDC])) {
            gotTDC[iTDC]=true;
            antani->Fill(iTDC, (prevVal-thisVal)/(thisX-prevX));
        } 
      } else firstVal=false;
      // myHistoDifferential[iTDC]->SetBinContent(iBin, prevVal-thisVal);
      prevVal = thisVal;
      prevX = thisX;
    }

    TCanvas* myCanvas;
    
    //myCanvas = new TCanvas();
    //myCanvas->cd();
    //myCanvas->SetLogy();

    myGraphDifferential[iTDC]->SetMarkerStyle(8);
    // myGraphDifferential[iTDC]->Draw("ap");

		//edit G. Auzinger: fit distribution for every TDC with a landau in range 30 to 200
//		TF1* myLandau = new TF1("fit", "TMath::Landau(x)", 30, 200);
//		myGraphDifferential[iTDC]->Fit("landau", "RM+","same", 50, 250);

    // myGraphDifferential[iTDC]->Fit(
    
    // myHistoDifferential[iTDC]->Rebin(2);
    // myHistoDifferential[iTDC]->Draw();
    
    // myCanvas = new TCanvas();
    // myCanvas->SetLogy();
    // myHistoIntegral[iTDC]->Draw();
    
  }

  new TCanvas();
  antani->Draw();
}
