/** \file Ornl2016Processor.cpp
 * \brief A class to process data from the Ornl 2016 OLTF experiment using
 * VANDLE. Using Root and Damm for histogram analysis. 
 * Moved to PAASS Oct 2016
 *
 *
 *\author S. V. Paulauskas
 *\date February 10, 2016
 *
 *\Edits by Thomas King 
 *\Starting June 2016
 *
 *
*/
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cmath>
#include <vector>

#include "DammPlotIds.hpp"
#include "DetectorDriver.hpp"

#include "DoubleBetaProcessor.hpp"
#include "GeProcessor.hpp"
#include "Ornl2016Processor.hpp"
#include "VandleProcessor.hpp"

static unsigned int evtNum = 0;

namespace dammIds {
    namespace experiment {
        const int D_VANDLEMULT = 0;
        const int DD_CTOFNOTAPE = 1;
        const int DD_QDCTOFNOGATE = 2;
        const int DD_CORTOFVSEGAM = 3;
        const int DD_QDCVSCORTOFMULT1 = 4;
        const int DD_MULT2SYM = 5;
        const int DD_LIGLEN = 6;
        const int DD_LIGLTOF = 7;

        const int D_LABR3SUM = 8;
        const int D_LABR3BETA = 9;
        const int D_HPGESUM = 10;
        const int D_HPGESUMBETA = 11;
        const int D_NAISUM = 12;
        const int D_NAIBETA = 13;

        const int DD_TOFVSNAI = 14;
        const int DD_TOFVSHAGRID = 15;
        const int DD_TOFVSGE = 16;

        const int D_BETASCALARRATE = 29; //6079 in his
        const int D_BETAENERGY = 30;
        const int DD_QDCVTOF = 31;

        const int D_DBGge = 35 ;//super beta gated specta;
        const int D_DBGnai = 36 ;//super beta gated specta;
        const int D_DBGlabr = 37 ;//super beta gated specta;
    }
}//namespace dammIds

using namespace std;
using namespace dammIds::experiment;

void Ornl2016Processor::DeclarePlots(void) {

    DeclareHistogram1D(D_VANDLEMULT, S7, "Vandle Multiplicity");
    DeclareHistogram2D(DD_QDCTOFNOGATE, SC, SD, "QDC ToF Ungated");
    DeclareHistogram2D(DD_QDCVSCORTOFMULT1, SC, SC, "QDC vs Cor Tof Mult1");
    DeclareHistogram2D(DD_LIGLEN, SC, S5, "E - LiGlass");
    DeclareHistogram2D(DD_LIGLTOF, SC, SC, "E vs ToF - LiGlass");

    DeclareHistogram1D(D_LABR3SUM, SC, "HAGRiD summed");
    DeclareHistogram1D(D_LABR3BETA, SC, "HAGRiD summed - BETA GATED");
    DeclareHistogram1D(D_NAISUM, SC, "NaI summed");
    DeclareHistogram1D(D_NAIBETA, SC, "NaI summed - BETA GATED");

    DeclareHistogram1D(D_HPGESUM, SC, "HPGe Clover summed");
    DeclareHistogram1D(D_HPGESUMBETA, SC, "HPGe Clover summed - BETA GATED");

    DeclareHistogram2D(DD_TOFVSNAI, SC, SB, "ToF vs. NaI");
    DeclareHistogram2D(DD_TOFVSHAGRID, SC, SB, "ToF vs. HAGRiD");
    DeclareHistogram2D(DD_TOFVSGE, SC, SB, "ToF vs. Ge");

    DeclareHistogram1D(D_BETASCALARRATE, SB, "Beta scalar per cycle");
    DeclareHistogram1D(D_BETAENERGY, SD, "Beta Energy");

    DeclareHistogram2D(DD_QDCVTOF, SC, SD, "exp processor made qdc vs tof");

    DeclareHistogram1D(D_DBGge, SD, "Double Beta gated HPGe Energy");
    DeclareHistogram1D(D_DBGnai, SD, "Double Beta gated NaI Energy");
    DeclareHistogram1D(D_DBGlabr, SD, "Double Beta gated LaBr3 Energy");


    // //Declaring beta gated
    // for (unsigned int i=0; i < 4; i++){
    //   stringstream ss;
    //   ss<< "HPGe " << i <<" - beta gated " ;
    //   DeclareHistogram1D(D_IGEBETA + i,SC,ss.str().c_str());
    // }

    // //Declaring NaI beta gated
    // for (unsigned int i=0; i < 10; i++){
    //   stringstream ss;
    //   ss<< "NaI " << i << " - beta gated ";
    //   DeclareHistogram1D(D_INAIBETA + i ,SC,ss.str().c_str());
    // }

    // //Declaring Hagrid beta gated
    // for (unsigned int i = 0; i < 16; i++){
    //   stringstream ss;
    //   ss<< "LaBr " << i << " - beta gated ";
    //   DeclareHistogram1D(D_IHAGBETA + i ,SC,ss.str().c_str());

    // }

}


/*void Ornl2016Processor::rootGstrutInit(RAY &strutName) { //Zeros the entire aux  structure

    fill(strutName.LaBr, strutName.LaBr + 16, 0);
    fill(strutName.NaI, strutName.NaI + 10, 0);
    fill(strutName.Ge, strutName.Ge + 4, 0);
    strutName.beta = -9999;
    strutName.cycle = -9999;
    strutName.eventNum = -9999;
    strutName.gMulti = -9999;
    strutName.nMulti = -9999;
    strutName.lMulti = -9999;
    strutName.bMulti = -9999;

}
*/
void Ornl2016Processor::rootGstrutInit2(PROSS &strutName) { //Zeros the entire processed structure
    strutName.AbE = -999;
    strutName.AbEvtNum = -9999;
    strutName.Multi = 0;
    //strutName.SymX = -999;
    //strutName.SymY = -999;
}


Ornl2016Processor::Ornl2016Processor() : EventProcessor(
        OFFSET, RANGE, "Ornl2016Processor") {

    debugging = to_bool(Globals::get()->GetOrnl2016Arguments().find("Debugging")->second);
    Pvandle = to_bool(Globals::get()->GetOrnl2016Arguments().find("MkVandle")->second);

    SupBetaWin=atof(Globals::get()->GetOrnl2016Arguments().find("SupBetaWin")->second.c_str());

    associatedTypes.insert("ge");
    associatedTypes.insert("nai");
    associatedTypes.insert("labr3");
    associatedTypes.insert("beta");

    if (Pvandle) {
        associatedTypes.insert("vandle");}

    LgammaThreshold_ = atof(Globals::get()->GetOrnl2016Arguments().find("gamma_threshold_L")->second.c_str());
    LsubEventWindow_ = atof(Globals::get()->GetOrnl2016Arguments().find("sub_event_L")->second.c_str());
    NgammaThreshold_ = atof(Globals::get()->GetOrnl2016Arguments().find("gamma_threshold_N")->second.c_str());
    NsubEventWindow_ = atof(Globals::get()->GetOrnl2016Arguments().find("sub_event_N")->second.c_str());
    GgammaThreshold_ = atof(Globals::get()->GetOrnl2016Arguments().find("gamma_threshold_G")->second.c_str());
    GsubEventWindow_ = atof(Globals::get()->GetOrnl2016Arguments().find("sub_event_G")->second.c_str());

    // initalize addback vectors
    LaddBack_.push_back(ScintAddBack(0, 0, 0));
    NaddBack_.push_back(ScintAddBack(0, 0, 0));
    GaddBack_.push_back(ScintAddBack(0, 0, 0));

    // ROOT file Naming
    string hisPath = Globals::get()->GetOutputPath();
    string hisfilename = hisPath + Globals::get()->GetOutputFileName();
    string rootname = hisfilename + "-gammaSing.root";
    string rootname2 = hisfilename + "-gammaAddBk.root";
    string rootname3 = hisfilename + "-histo.root";
    string rootname4 = hisfilename + "-vandleDebug.root";
    string rootname5 = hisfilename + "-vandle.root";

    // Start Primary Root File
    rootFName_ = new TFile(rootname.c_str(), "RECREATE");
    Taux = new TTree("Taux", "Tree for Gamma-ray stuff @ ORNL2016");

    //Taux Stuff
    //singBranch = Taux->Branch("sing", &sing,"LaBr[16]/D:NaI[10]/D:Ge[4]/D:beta/D:eventNum/D:cycle/i:gMulti/i:nMulti/i:hMulti/i:bMulti/i");
    //rootGstrutInit(sing);

    Taux->Branch("aux_LaBrEn",&aux_LaBrEn);
    Taux->Branch("aux_LaBrNum",&aux_LaBrNum);
    Taux->Branch("aux_LaBrTime",&aux_LaBrTime);
    Taux->Branch("aux_NaIEn",&aux_NaIEn);
    Taux->Branch("aux_NaINum",&aux_NaINum);
    Taux->Branch("aux_NaITime",&aux_NaITime);
    Taux->Branch("aux_GeEn",&aux_GeEn);
    Taux->Branch("aux_GeNum",&aux_GeNum);
    Taux->Branch("aux_GeTime",&aux_GeTime);
    Taux->Branch("aux_betaEn",&aux_betaEn);
    Taux->Branch("aux_betaNum",&aux_betaNum);
    Taux->Branch("aux_betaTime",&aux_betaTime);
    Taux->Branch("aux_cycle",&aux_cycle);
    Taux->Branch("aux_geTdiff",&geTdiff);
    Taux->Branch("aux_naiTdiff",&naiTdiff);
    Taux->Branch("aux_labrTdiff",&labrTdiff);
    Taux->Branch("aux_eventNum",&aux_eventNum);
    Taux->Branch("aux_gMulti",&aux_gMulti);
    Taux->Branch("aux_nMulti",&aux_nMulti);
    Taux->Branch("aux_lMulti",&aux_lMulti);
    Taux->Branch("aux_bMulti",&aux_bMulti);


    Taux->SetAutoFlush(3000);

    rootFName2_ = new TFile(rootname2.c_str(), "RECREATE");
    Tadd = new TTree("Tadd", "Tree for Addbacks @ ORNL2016");

    gProcBranch = Taux->Branch("Gpro", &Gpro, "AbE/D:AbEvtNum/D:Multi/D");
    lProcBranch = Taux->Branch("Lpro", &Lpro, "AbE/D:AbEvtNum/D:Multi/D");
    nProcBranch = Taux->Branch("Npro", &Npro, "AbE/D:AbEvtNum/D:Multi/D");

    rootGstrutInit2(Gpro);
    rootGstrutInit2(Lpro);
    rootGstrutInit2(Npro);

    // End second Root File
    if (Pvandle) {
        // Start Third  RootFile
        rootFName3_ = new TFile(rootname3.c_str(), "RECREATE");
        qdcVtof_ = new TH2D("qdcVtof", "", 1000, -100, 900, 32000, -16000, 16000);
        tofVGe_ = new TH2D("tofVGe", "", 1500, -100, 1400, 16000, 0, 16000);
        tofVLabr_ = new TH2D("tofVLaBr", "", 1500, -100, 1400, 16000, 0, 16000);
        tofVNai_ = new TH2D("tofVNaI", "", 1500, -100, 1400, 16000, 0, 16000);

        // End (Histo) RootFile
    }

    // Start fourth debugging RootFille
    if (debugging) {
        rootFName4_ = new TFile(rootname4.c_str(), "RECREATE");
        Wave = new TTree("Wave", "Tree for Waveform Analyzer Debugging");

        Wave->Branch("evtNumber", &evtNumber);
        Wave->Branch("output_name", &output_name);
        Wave->Branch("vandle_subtype", &vandle_subtype);
        Wave->Branch("vandle_BarQDC", &vandle_BarQDC);
        Wave->Branch("vandle_lQDC", &vandle_lQDC);
        Wave->Branch("vandle_rQDC", &vandle_rQDC);
        Wave->Branch("vandle_QDCPos", &vandle_QDCPos);
        Wave->Branch("vandle_TOF", &vandle_TOF);
        Wave->Branch("vandle_lSnR", &vandle_lSnR);
        Wave->Branch("vandle_rSnR", &vandle_rSnR);
        Wave->Branch("vandle_lAmp", &vandle_lAmp);
        Wave->Branch("vandle_rAmp", &vandle_rAmp);
        Wave->Branch("vandle_lMaxAmpPos", &vandle_lMaxAmpPos);
        Wave->Branch("vandle_rMaxAmpPos", &vandle_rMaxAmpPos);
        Wave->Branch("vandle_lAveBaseline", &vandle_lAveBaseline);
        Wave->Branch("vandle_rAveBaseline", &vandle_rAveBaseline);
        Wave->Branch("vandle_barNum", &vandle_barNum);
        Wave->Branch("vandle_TAvg", &vandle_TAvg);
        Wave->Branch("vandle_Corrected_TAvg", &vandle_Corrected_TAvg);
        Wave->Branch("vandle_TDiff", &vandle_TDiff);
        Wave->Branch("vandle_Corrected_TDiff", &vandle_Corrected_TDiff);
        Wave->Branch("vandle_ltrace", &vandle_ltrace);
        Wave->Branch("vandle_rtrace", &vandle_rtrace);
        Wave->Branch("beta_BarQDC", &beta_BarQDC);
        Wave->Branch("beta_lQDC", &beta_lQDC);
        Wave->Branch("beta_rQDC", &beta_rQDC);
        Wave->Branch("beta_lSnR", &beta_lSnR);
        Wave->Branch("beta_rSnR", &beta_rSnR);
        Wave->Branch("beta_lAmp", &beta_lAmp);
        Wave->Branch("beta_rAmp", &beta_rAmp);
        Wave->Branch("beta_lMaxAmpPos", &beta_lMaxAmpPos);
        Wave->Branch("beta_rMaxAmpPos", &beta_rMaxAmpPos);
        Wave->Branch("beta_lAveBaseline", &vandle_lAveBaseline);
        Wave->Branch("beta_rAveBaseline", &vandle_rAveBaseline);
        Wave->Branch("beta_barNum", &beta_barNum);
        Wave->Branch("beta_TAvg", &beta_TAvg);
        Wave->Branch("beta_Corrected_TAvg", &beta_Corrected_TAvg);
        Wave->Branch("beta_TDiff", &beta_TDiff);
        Wave->Branch("beta_Corrected_TDiff", &beta_Corrected_TDiff);
        Wave->Branch("beta_ltrace", &beta_ltrace);
        Wave->Branch("beta_rtrace", &beta_rtrace);


        Wave->SetAutoFlush(3000);
        //End debugging RootFille
    }

    // Tvan Stuff
    if (Pvandle) {
        rootFName5_ = new TFile(rootname5.c_str(), "RECREATE");
        Tvan = new TTree("Tvan", "Tree for Vandle Stuff (coincident gammas as well) @ ORNL2016");

        Tvan->Branch("evtNumber", &evtNumber);
        Tvan->Branch("output_name", &output_name);

        Tvan->Branch("vandle_BarQDC", &vandle_BarQDC);
        Tvan->Branch("vandle_barNum", &vandle_barNum);
        Tvan->Branch("vandle_TOF", &vandle_TOF);
        Tvan->Branch("vandle_TAvg", &vandle_TAvg);
        Tvan->Branch("vandle_Corrected_TAvg", &vandle_Corrected_TAvg);
        Tvan->Branch("vandle_TDiff", &vandle_TDiff);

        Tvan->Branch("vandle_QDCPos", &vandle_QDCPos);
        Tvan->Branch("vandle_lSnR", &vandle_lSnR);
        Tvan->Branch("vandle_rSnR", &vandle_rSnR);
        Tvan->Branch("vandle_lAmp", &vandle_lAmp);
        Tvan->Branch("vandle_rAmp", &vandle_rAmp);
        Tvan->Branch("vandle_lMaxAmpPos", &vandle_lMaxAmpPos);
        Tvan->Branch("vandle_rMaxAmpPos", &vandle_rMaxAmpPos);
        Tvan->Branch("vandle_lAveBaseline", &vandle_lAveBaseline);
        Tvan->Branch("vandle_rAveBaseline", &vandle_rAveBaseline);
        Tvan->Branch("vandle_ltrace", &vandle_ltrace);
        Tvan->Branch("vandle_rtrace", &vandle_rtrace);

        Tvan->Branch("vandle_ge", &vandle_ge);
        Tvan->Branch("vandle_labr3", &vandle_labr3);
        Tvan->Branch("vandle_nai", &vandle_nai);

        Tvan->SetAutoFlush(3000);

    }

}


Ornl2016Processor::~Ornl2016Processor() {

    //sing
    rootFName_->Write();
    rootFName_->Close();
    delete (rootFName_);

    //addback
    rootFName2_->Write();
    rootFName2_->Close();
    delete (rootFName2_);

    //Wave Debugging
    if (debugging) {
        rootFName4_->Write();
        rootFName4_->Close();
        delete (rootFName4_);
    }

    if (Pvandle) {
        //histo
        rootFName3_->Write();
        rootFName3_->Close();
        delete (rootFName3_);

        //Vandle + coincidence
        rootFName5_->Write();
        rootFName5_->Close();
        delete (rootFName5_);
    }

}


bool Ornl2016Processor::PreProcess(RawEvent &event) {
    if (!EventProcessor::PreProcess(event))
        return (false);


    EndProcess();
    return (true);
}

bool Ornl2016Processor::Process(RawEvent &event) {
    if (!EventProcessor::Process(event))
        return (false);
    double plotOffset_ = 200;
    double plotMult_ = 2;

    map<unsigned int, pair<double, double> > lrtBetas;
    BarMap betas, vbars;

    hasBeta = false;
    hasBeta = TreeCorrelator::get()->place(
            "Beta")->status(); //might need a static initialize to false + reset at the end

    if ((Pvandle || debugging) && event.GetSummary("vandle")->GetList().size() != 0) {
        vbars = ((VandleProcessor *) DetectorDriver::get()->GetProcessor("VandleProcessor"))->GetBars();
    }

    if (event.GetSummary("beta:double")->GetList().size() != 0) {
        betas = ((DoubleBetaProcessor *) DetectorDriver::get()->GetProcessor("DoubleBetaProcessor"))->GetBars();
        lrtBetas = ((DoubleBetaProcessor *) DetectorDriver::get()->GetProcessor(
                "DoubleBetaProcessor"))->GetLowResBars();
    }

    static const vector<ChanEvent *> &labr3Evts = event.GetSummary("labr3")->GetList();
    static const vector<ChanEvent *> &naiEvts = event.GetSummary("nai")->GetList();
    static const vector<ChanEvent *> &geEvts = event.GetSummary("ge")->GetList();


    /// PLOT ANALYSIS HISTOGRAMS-------------------------------------------------------------------------------------------------------------------------------------

    // initalize the root structures
    //rootGstrutInit(sing);
    rootGstrutInit2(Gpro);
    rootGstrutInit2(Lpro);
    rootGstrutInit2(Npro);


    //Setting vars for addback
    double LrefTime = -2.0 * LsubEventWindow_;
    double NrefTime = -2.0 * NsubEventWindow_;
    double GrefTime = -2.0 * GsubEventWindow_;

    //Cycle timing
    static double cycleLast = 2;
    static int cycleNum = 0;

    if (TreeCorrelator::get()->place("Cycle")->status()) {
        double cycleTime = TreeCorrelator::get()->place("Cycle")->last().time;
        cycleTime *= Globals::get()->GetClockInSeconds() * 1.e9;
        if (cycleTime != cycleLast) {
            double tdiff = (cycleTime - cycleLast) / 1e6; //Outputs cycle length in msecs.
            if (cycleNum == 0) {
                cout
                        << " #  There are some events at the beginning of the first segment missing from Histograms that use cycleNum."
                        << endl << " #  This is a product of not starting the cycle After the LDF." << endl
                        << " #  This First TDIFF is most likely nonsense" << endl;
            }
            cycleLast = cycleTime;
            cycleNum++;
            cout << "Cycle Change " << endl << "Tdiff (Cycle start and Now) (ms)= " << tdiff << endl
                 << "Starting on Cycle #" << cycleNum << endl;
        }
    }
    aux_cycle = cycleNum;

    //set multiplicys for sing branch based on the size of the detector maps for the event. limitation: sub event is smaller than full event this will end up being too large
    aux_gMulti = geEvts.size();
    aux_nMulti = naiEvts.size();
    aux_lMulti = labr3Evts.size();
    aux_bMulti = lrtBetas.size();


    for (map<unsigned int, pair<double, double> >::iterator bIt = lrtBetas.begin(); bIt != lrtBetas.end(); bIt++) {

        aux_betaTime = betaSubTime = bIt->second.first;
        plot(D_BETASCALARRATE, cycleNum);//PLOTTING BETA SCALAR SUM per CYCLE (LIKE 759 but per cycle vs per second
        plot(D_BETAENERGY, bIt->second.second);
        aux_betaEn = bIt->second.second;
        aux_betaNum = bIt->first;
    }


    //NaI ONLY
    // ----------------------------------------------------------------------------------------------------------
    for (vector<ChanEvent *>::const_iterator itNai = naiEvts.begin();
         itNai != naiEvts.end(); itNai++) {
        int naiNum = (*itNai)->GetChannelNumber();
        aux_NaINum = naiNum;
        aux_NaIEn = (*itNai)->GetCalibratedEnergy();
        aux_NaITime = naiEvtTime = (*itNai)->GetTime();
        //sing.NaI[naiNum] = (*itNai)->GetCalibratedEnergy();
        plot(D_NAISUM, (*itNai)->GetCalibratedEnergy()); //plot totals
        aux_naiTdiff = abs(naiEvtTime-betaSubTime);

        if (aux_naiTdiff < SupBetaWin) {
            plot(D_DBGnai,(*itNai)->GetCalibratedEnergy());
        }
//Beta Gate and addback
        if (hasBeta) {  //Beta Gate
            plot(D_NAIBETA, (*itNai)->GetCalibratedEnergy()); //plot beta-gated totals

            //begin addback calulations for NaI
            double energy = (*itNai)->GetCalibratedEnergy();
            double time = (*itNai)->GetTime();

            if (energy < NgammaThreshold_) {
                continue;
            }//end energy comp if statment
            double t1 = Globals::get()->GetClockInSeconds();
            double dtime = abs(time - NrefTime) * t1;

            if (dtime >
                NsubEventWindow_) { //if event time is outside sub event window start new addback after filling tree
                Npro.AbEvtNum = evtNum;
                Npro.AbE = NaddBack_.back().energy;
                Npro.Multi = NaddBack_.back().multiplicity;
                Taux->Fill();
                NaddBack_.push_back(ScintAddBack());
            }//end subEvent IF
            NaddBack_.back().energy += energy; // if still inside sub window: incrament
            NaddBack_.back().time = time;
            NaddBack_.back().multiplicity += 1;
            NrefTime = time;

        }//end beta gate
    } //NaI loop End

    //HPGe ONLY---------------------------------------------------------------------------------------------------------------------------------------------
    for (vector<ChanEvent *>::const_iterator itGe = geEvts.begin();
         itGe != geEvts.end(); itGe++) {
        int geNum = (*itGe)->GetChanID().GetLocation();
        aux_GeEn = (*itGe)->GetCalibratedEnergy();
        aux_GeNum = geNum;
        aux_GeTime = geEvtTime= (*itGe)->GetTime();

        aux_geTdiff = abs(betaSubTime-geEvtTime);
        if (aux_geTdiff < SupBetaWin){
            plot(D_DBGge,(*itGe)->GetCalibratedEnergy());
        }

        //sing.Ge[geNum] = (*itGe)->GetCalibratedEnergy();
        plot(D_HPGESUM, (*itGe)->GetCalibratedEnergy()); //plot non-gated totals

        if (hasBeta) { //beta-gated Processing to cut LaBr contamination out
            plot(D_HPGESUMBETA, (*itGe)->GetCalibratedEnergy()); //plot non-gated totals
            //begin addback calulations for clover
            double energy = (*itGe)->GetCalibratedEnergy();
            double time = (*itGe)->GetTime();
            if (energy < GgammaThreshold_) {
                continue;
            }//end energy comp if statment
            double t1 = Globals::get()->GetClockInSeconds();
            double dtime = abs(time - GrefTime) * t1;

            if (dtime >
                GsubEventWindow_) { //if event time is outside sub event window start new addback after filling tree
                Gpro.AbEvtNum = evtNum;
                Gpro.Multi = GaddBack_.back().multiplicity;
                Gpro.AbE = GaddBack_.back().energy;
                Taux->Fill();
                GaddBack_.push_back(ScintAddBack());
            } //end subEvent IF

            GaddBack_.back().energy += energy;
            GaddBack_.back().time = time;
            GaddBack_.back().multiplicity += 1;
            GrefTime = time;


        } //end BetaGate
    } //GE loop end

    //HAGRiD ONLY-------------------------------------------------------------------------------------------------------------------------------------------
    for (vector<ChanEvent *>::const_iterator itLabr = labr3Evts.begin();
         itLabr != labr3Evts.end(); itLabr++) {
        int labrNum = (*itLabr)->GetChanID().GetLocation();
        plot(D_LABR3SUM, (*itLabr)->GetCalibratedEnergy()); //plot non-gated totals
        //sing.LaBr[labrNum] = (*itLabr)->GetCalibratedEnergy();

        aux_LaBrNum = labrNum;
        aux_LaBrEn = (*itLabr)->GetCalibratedEnergy();
        aux_LaBrTime =labrEvtTime = (*itLabr)->GetTime();
        aux_labrTdiff = abs(betaSubTime-labrEvtTime);

        if (aux_labrTdiff < SupBetaWin){
            plot(D_DBGlabr,(*itLabr)->GetCalibratedEnergy());
        }


        if (hasBeta) {

            plot(D_LABR3BETA, (*itLabr)->GetCalibratedEnergy()); //plot beta-gated totals
            //begin addback calculations for LaBr | Beta Gated to Remove La Contamination

            double energy = (*itLabr)->GetCalibratedEnergy();
            double time = (*itLabr)->GetTime();

            if (energy < LgammaThreshold_) {
                continue;
            }//end energy comp if statment

            double t1 = Globals::get()->GetClockInSeconds();
            double dtime = abs(time - LrefTime) * t1;

            if (dtime >
                LsubEventWindow_) { //if event time is outside sub event window start new addback after filling tree

                Lpro.AbEvtNum = evtNum;
                Lpro.AbE = LaddBack_.back().energy;
                Lpro.Multi = LaddBack_.back().multiplicity;
                Taux->Fill();
                LaddBack_.push_back(ScintAddBack());
            }// end if for new entry in vector

            LaddBack_.back().energy += energy;
            LaddBack_.back().time = time;
            LaddBack_.back().multiplicity += 1;
            LrefTime = time;


            // Left in for referance but not usefull without more than 1 detc per type (like 2 clovers)
//                for (vector<ChanEvent *>::const_iterator itLabr2 = itLabr + 1;
//                     itLabr2 != labr3Evts.end(); itLabr2++) {
//                    double energy2 = (*itLabr2)->GetCalEnergy();
//                    int labrNum2 = (*itLabr2)->GetChanID().GetLocation();
//                    //double time2=(*itGe2)->GetCorrectedTime();
//                    if (energy2 < LgammaThreshold_) {
//                        continue;
//                    }//end energy comp if statement
//                    if (labrNum2 != labrNum) {
//                        Lpro.SymX = energy;
//                        Lpro.SymY = energy2;
//                        Taux->Fill();
//                    }
//
//
//                } //end symplot inner loop

        }//end beta gate


    } //Hagrid loop end

     //Begin VANDLE
     for (BarMap::iterator it = vbars.begin(); it != vbars.end(); it++) {
         TimingDefs::TimingIdentifier barId = (*it).first;
         BarDetector bar = (*it).second;


         if (!bar.GetHasEvent() || bar.GetType() == "small")
             continue;

         int barLoc = barId.first;
         TimingCalibration cal = bar.GetCalibration();

         for (BarMap::iterator itStart = betas.begin();
              itStart != betas.end(); itStart++) {
             unsigned int startLoc = (*itStart).first.first;
             BarDetector start = (*itStart).second;
             if (!start.GetHasEvent())
                 continue;

             double tof = bar.GetTimeAverage() - start.GetTimeAverage() + cal.GetTofOffset(startLoc);

             double corTof = ((VandleProcessor *) DetectorDriver::get()->GetProcessor("VandleProcessor"))->
                     CorrectTOF(tof, bar.GetFlightPath(), cal.GetZ0());

             plot(DD_QDCVTOF, (tof * 2) + plotOffset_, bar.GetQdc());

             qdcVtof_->Fill(tof,bar.GetQdc());

             vandle_subtype = bar.GetType();
             vandle_lSnR = bar.GetLeftSide().GetTrace().GetSignalToNoiseRatio();
             vandle_rSnR = bar.GetRightSide().GetTrace().GetSignalToNoiseRatio();
             vandle_lAmp = bar.GetLeftSide().GetMaximumValue();
             vandle_rAmp = bar.GetRightSide().GetMaximumValue();
             vandle_lMaxAmpPos = bar.GetLeftSide().GetMaximumPosition();
             vandle_rMaxAmpPos = bar.GetRightSide().GetMaximumPosition();
             vandle_lAveBaseline = bar.GetLeftSide().GetAveBaseline();
             vandle_rAveBaseline = bar.GetRightSide().GetAveBaseline();
             vandle_BarQDC = bar.GetQdc();
             vandle_QDCPos = bar.GetQdcPosition();
             vandle_lQDC = bar.GetLeftSide().GetTraceQdc();
             vandle_rQDC = bar.GetRightSide().GetTraceQdc();
             vandle_TOF = tof;
             vandle_barNum = barLoc;
             vandle_TAvg = bar.GetTimeAverage();
             vandle_Corrected_TAvg = bar.GetCorTimeAve();
             vandle_TDiff = bar.GetTimeDifference();
             vandle_Corrected_TDiff = bar.GetCorTimeDiff();
             vandle_ltrace = bar.GetLeftSide().GetTrace();
             vandle_rtrace = bar.GetRightSide().GetTrace();

             beta_lSnR = start.GetLeftSide().GetTrace().GetSignalToNoiseRatio();
             beta_rSnR = start.GetRightSide().GetTrace().GetSignalToNoiseRatio();
             beta_lAmp = start.GetLeftSide().GetMaximumValue();
             beta_rAmp = start.GetRightSide().GetMaximumValue();
             beta_lMaxAmpPos = start.GetLeftSide().GetMaximumPosition();
             beta_rMaxAmpPos = start.GetRightSide().GetMaximumPosition();
             beta_lAveBaseline = start.GetLeftSide().GetAveBaseline();
             beta_rAveBaseline = start.GetRightSide().GetAveBaseline();
             beta_BarQDC = start.GetQdc();
             beta_lQDC = start.GetLeftSide().GetTraceQdc();
             beta_rQDC = start.GetRightSide().GetTraceQdc();
             beta_barNum = startLoc;
             beta_TAvg = start.GetTimeAverage();
             beta_Corrected_TAvg = start.GetCorTimeAve();
             beta_TDiff = start.GetTimeDifference();
             beta_Corrected_TDiff = start.GetCorTimeDiff();
             beta_ltrace = start.GetLeftSide().GetTrace();
             beta_rtrace = start.GetRightSide().GetTrace();


             //TracePloting commented
/*          int itTVl=0;
             for (vector<unsigned int>::const_iterator itTL = bar.GetLeftSide().GetTrace().begin();
                  itTL != bar.GetLeftSide().GetTrace().end(); itTL++) {
                 Vwave.Ltrace[itTVl]=(*itTL);
                 itTVl++;
             };

             int itTVr=0;
             for (vector<unsigned int>::const_iterator itTR = bar.GetRightSide().GetTrace().begin();
                  itTR != bar.GetRightSide().GetTrace().end(); itTR++) {
                 Vwave.Rtrace[itTVr]=(*itTR);
                 itTVr++;
             };

             int itTBl=0;
             for (vector<unsigned int>::const_iterator itTL = start.GetLeftSide().GetTrace().begin();
                  itTL != start.GetLeftSide().GetTrace().end(); itTL++) {
                 Bwave.Ltrace[itTBl]=(*itTL);
                 itTBl++;
             };

             int itTr=0;
             for (vector<unsigned int>::const_iterator itTR = start.GetRightSide().GetTrace().begin();
                  itTR != start.GetRightSide().GetTrace().end(); itTR++) {
                 Bwave.Rtrace[itTr]=(*itTR);
                 itTr++;
             };
*/


             //this is ghost flash troubleshooting code
 /*

             if (barLoc <8 || barLoc > 15){
                 plot(DD_QDCVTOFNOMOD2,(tof * 2) + plotOffset_, bar.GetQdc());
                 plot(D_MOD2CHECK,barLoc);
             };


             static int trcCounter = 0;
             static int ftrcCounter = 0;
             double dammBin = (tof * 2) + 1000;
             static int badTrcEvtCounter = 0;
             if (dammBin >= 1048 && dammBin <= 1078) {
                 for (vector<unsigned int>::const_iterator itTL = bar.GetLeftSide().GetTrace()->begin();
                      itTL != bar.GetLeftSide().GetTrace()->end(); itTL++) {
                     plot(DD_FLASHTRACES, itTL - bar.GetLeftSide().GetTrace()->begin(), ftrcCounter, (*itTL));
                 }
                 for (vector<unsigned int>::const_iterator itTR = bar.GetRightSide().GetTrace()->begin();
                      itTR != bar.GetRightSide().GetTrace()->end(); itTR++) {
                     plot(DD_FLASHTRACES, itTR - bar.GetRightSide().GetTrace()->begin(),
                          ftrcCounter + 1, (*itTR));
                 }
                 ftrcCounter += 3;

                 plot(D_BADLOCATION, barLoc);
                 plot(D_STARTLOC,startLoc);
                 if (TreeCorrelator::get()->place("Cycle")->status()) {
                     double inCycleTime = bar.GetTimeAverage();
                     double cycleTimeLast = TreeCorrelator::get()->place("Cycle")->last().time;
                     cycleTimeLast *= (Globals::get()->clockInSeconds() * 1.e9);
                     double currenttime = (inCycleTime) - (cycleTimeLast);


                     //cout << endl << "timeavg=" << inCycleTime << endl << "currenttime=" << currenttime << endl
                     //     << "last cycle start=" << cycleTimeLast << endl;

                     plot(DD_SIGNOIS, bar.GetLeftSide().GetSignalToNoiseRatio(),bar.GetQdc() );
                     plot(DD_ETRIGVSQDC,start.GetQdc(),bar.GetQdc());
                     plot(D_BADCYCLE, cycleNum);
                     plot(DD_BADCYCLELOC,cycleNum,barLoc);
                 }
             }
             else if (dammBin >=995 && dammBin<=1015){
                 plot(D_GCYCLE,cycleNum);
             }
             else {

                 plot(D_STARTLOC,startLoc+1000);

                 for (vector<unsigned int>::const_iterator itTL = bar.GetLeftSide().GetTrace()->begin();
                      itTL != bar.GetLeftSide().GetTrace()->end(); itTL++) {
                     plot(DD_TRACES, itTL - bar.GetLeftSide().GetTrace()->begin(),
                          trcCounter, (*itTL));
                 }

                 for (vector<unsigned int>::const_iterator itTR = bar.GetRightSide().GetTrace()->begin();
                      itTR != bar.GetRightSide().GetTrace()->end(); itTR++) {
                     plot(DD_TRACES, itTR - bar.GetRightSide().GetTrace()->begin(),
                          trcCounter + 1, (*itTR));
                 }

                 trcCounter += 3;
                 plot(DD_QDCVSTOFNOF, (tof * 2) + 1000, bar.GetQdc());
                 plot(DD_GSIGNOIS, bar.GetLeftSide().GetSignalToNoiseRatio(),bar.GetQdc() );
                 plot(DD_GETRIGVSQDC,start.GetQdc(),bar.GetQdc());
             }


             */

             //tof vs gammas in damm for testing against root when its working right
             //Gamma Loops for VANDLE
             //labr loop for mVan
             int labrNum;
             double labrEn;
             for (vector<ChanEvent *>::const_iterator itlabr3 = labr3Evts.begin();
                  itlabr3 != labr3Evts.end(); itlabr3++) {
                 labrNum = (*itlabr3)->GetChanID().GetLocation();
                 labrEn = (*itlabr3)->GetCalibratedEnergy();
                 plot(DD_TOFVSHAGRID, labrEn, tof * plotMult_ + 200);

                 tofVLabr_->Fill(labrEn,tof);

                 vandle_labr3.push_back(make_pair((double)labrNum,labrEn));
             };

             //Nai loop for mVan
             int naiNum;
             double naiEn;
             for (vector<ChanEvent *>::const_iterator itNai = naiEvts.begin();
                  itNai != naiEvts.end(); itNai++) {
                 naiNum = (*itNai)->GetChanID().GetLocation();
                 naiEn = (*itNai)->GetCalibratedEnergy();
                 plot(DD_TOFVSNAI, naiEn, tof * plotMult_ + 200);

                 tofVNai_->Fill(naiEn,tof);

                 vandle_nai.push_back(make_pair((double)naiNum,naiEn));
             };

             //ge loop for mVan
             int geNum;
             double geEn;
             for (vector<ChanEvent *>::const_iterator itGe = geEvts.begin();
                  itGe != geEvts.end(); itGe++) {
                 geNum = (*itGe)->GetChanID().GetLocation();
                 geEn = (*itGe)->GetCalibratedEnergy();
                 plot(DD_TOFVSGE, geEn, tof * plotMult_ + 200);

                 tofVGe_->Fill(geEn,tof);
                 vandle_ge.push_back(make_pair((double)geNum,geEn));
             };
         };

         Wave->Fill();
         Tvan->Fill();

     };//End VANDLE

    evtNumber = evtNum;
    aux_eventNum = evtNum;

    evtNum++;
    EndProcess();
    return (true);
}