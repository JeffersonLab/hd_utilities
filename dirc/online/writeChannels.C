
class pixel {
    
public:
    int mod, chan, slot, fiber, modrow, modcol, lv, gain;
    double effic, scale;
    
    pixel(int locMod, int locChan, int locSlot, int locFiber, int locRow, int locColumn, int locLV, int locGain, double locEffic, double locScale);
};

pixel::pixel(int locMod, int locChan, int locSlot, int locFiber, int locRow, int locColumn, int locLV, int locGain, double locEffic, double locScale) {
    mod = locMod;
    chan = locChan;
    slot = locSlot;
    fiber = locFiber;
    modrow = locRow;
    modcol = locColumn;
    lv = locLV;
    gain = locGain;
    effic = locEffic;
    scale = locScale;
}

void writeChannels(int run = 71311, int DAC = 100, bool unityGain = false, bool minGain = false, bool equalizeTOT = false) {
   
    gStyle->SetOptStat(11111111);
    gSystem->Exec("mkdir -p maps");
    gSystem->Exec("mkdir -p config");
    
    TCanvas *cc = new TCanvas("cc", "cc", 1000, 800);
    cc->Divide(2,2);
    
    TH1F *hScale = new TH1F("hScale", "hScale; scale", 600, 0., 600.);
    TH1F *hGain = new TH1F("hGain", "hGain; gain", 256, 0., 256.);
    TH1F *hEffic = new TH1F("hEffic", "hEffic; efficiency", 100, 0, 1.5);
    TH1F *hPedestalMean = new TH1F("hPedestalMean", "hPedestalMean; pedestal", 400, 300, 700);
    TH1F *hPedestalWidth = new TH1F("hPedestalWidth", "hPedestalWidth; pedestal width", 150, 0, 15);
    TH1F *hEqualizedGain[2];
    hEqualizedGain[0] = new TH1F("hEqualizedGainNorth", "hEqualizedGainNorth; channel; gain", 6912, 0., 6912.);
    hEqualizedGain[1] = new TH1F("hEqualizedGainSouth", "hEqualizedGainSouth; channel; gain", 6912, 0., 6912.);
    
    // 2D channel maps
    TH2F *hGlobalPixelMap[2];
    TH2F *hScaleMap[2];
    TH2F *hGainMap[2];
    TH2F *hEfficMap[2];
    TH2F *hSSPslotMap[2];
    TH2F *hSSPfiberMap[2];
    TH2F *hFiberDAQmonMap[2];
    TH2F *hLVGroupMap[2];
    TH2F *hLaserModuleMap[2];
    TH2F *hPedestalMeanMap[2];
    TH2F *hPedestalSigmaMap[2];
    
    // 2D channel maps separated by box
    for(int ibox=0; ibox<2; ibox++) {
        hGlobalPixelMap[ibox] = new TH2F(Form("hGlobalPixelMap_%d", ibox), "hGlobalPixelMap; pixel row; pixel column", 144, -0.5, 143.5, 48, -0.5, 47.5);
        hScaleMap[ibox] = new TH2F(Form("hScaleMap_%d", ibox), "hScaleMap; pixel row; pixel column", 144, -0.5, 143.5, 48, -0.5, 47.5);
        hGainMap[ibox] = new TH2F(Form("hGainMap_%d", ibox), "hGainMap; pixel row; pixel column", 144, -0.5, 143.5, 48, -0.5, 47.5);
        hEfficMap[ibox] = new TH2F(Form("hEfficMap_%d", ibox), "hEfficMap; pixel row; pixel column", 144, -0.5, 143.5, 48, -0.5, 47.5);
        hSSPslotMap[ibox] = new TH2F(Form("hSSPslotMap_%d", ibox), "hSSPslotMap; pixel row; pixel column", 144, -0.5, 143.5, 48, -0.5, 47.5);
        hSSPfiberMap[ibox] = new TH2F(Form("hSSPfiberMap_%d", ibox), "hSSPfiberMap; pixel row; pixel column", 144, -0.5, 143.5, 48, -0.5, 47.5);
        hFiberDAQmonMap[ibox] = new TH2F(Form("hFiberDAQmonMap_%d", ibox), "hFiberDAQmonMap; pixel row; pixel column", 144, -0.5, 143.5, 48, -0.5, 47.5);
        hLVGroupMap[ibox] = new TH2F(Form("hFiberDAQmonMap_%d", ibox), "hFiberDAQmonMap; pixel row; pixel column", 144, -0.5, 143.5, 48, -0.5, 47.5);
        hLaserModuleMap[ibox] = new TH2F(Form("hLaserModuleMap_%d", ibox), "hLaserModuleMap; pixel row; pixel column", 144, -0.5, 143.5, 48, -0.5, 47.5);
        
        hPedestalMeanMap[ibox] = new TH2F(Form("hPedestalMeanMap_%d", ibox), "hPedestalMeanMap; pixel row; pixel column", 144, -0.5, 143.5, 48, -0.5, 47.5);
        hPedestalSigmaMap[ibox] = new TH2F(Form("hPedestalSigmaMap_%d", ibox), "hPedestalSigmaMap; pixel row; pixel column", 144, -0.5, 143.5, 48, -0.5, 47.5);
    }
    
    const int maxModules = 79;
    const int maxChannels = 192;
    vector<pixel> pixels;
    
    int pixelSizePrevious = 0;
    vector<int> unusedModules;
    
    ifstream inFile;
    
    for(int imod=0; imod<maxModules; imod++) {
        cout << "Displaying module " << imod+1 << endl;
        
        inFile.open(Form("run_26_text/module_%02d.txt", imod+1));
        if (!inFile) {
            cout << "Unable to open file";
            exit(1); // terminate with error
        }
        
        //<pedestal mean> " " <pedestal width> " " <integral above 5*width> " " <scale> " " <mu> " " <chi^2/ndf>
        
        int channel, pedestalMean, integral5sigma;
        double pedestalWidth, scale, mu, chi2ndf;
        while (inFile >> channel >> pedestalMean >> pedestalWidth >> integral5sigma >> scale >> mu >> chi2ndf) {
	    // cout << channel << " " << scale << endl;
            
            if(scale < 1 && mu != 0) { // hard code channels where fits didn't converge for low scales
                scale = 60;
            }
            
            double relEffic = integral5sigma/25580.;
            int gain = (int)(64 * 220/scale);
            
            // set maximum amplification
            if(gain > 255)
                gain = 255;
            
            if(gain < 64 && minGain)
                gain = 64;
            
            // give dummies a zero gain and efficiency
            if(chi2ndf == 0 && mu == 0) {
                relEffic = 0;
                gain = 0;
            }
            else { // fill histograms for monitoring
                //gain = 64.;
                hScale->Fill(scale);
                hGain->Fill(gain);
                hEffic->Fill(relEffic);
                hPedestalMean->Fill(pedestalMean);
                hPedestalWidth->Fill(pedestalWidth);
            }
            
            // set values for calibration tables by module
            int module = imod+1;
            
            // mod, chan, slot, fiber, modrow, modcol, lv, gain, pixel
            
            // North OB
            if(module==6)  pixels.push_back(pixel(module, channel, 3,  0,  0, 0, 1, gain, relEffic, scale));
            if(module==26) pixels.push_back(pixel(module, channel, 3,  1,  1, 0, 1, gain, relEffic, scale));
            if(module==24) pixels.push_back(pixel(module, channel, 3,  2,  2, 0, 1, gain, relEffic, scale));
            if(module==18) pixels.push_back(pixel(module, channel, 3,  3,  3, 0, 1, gain, relEffic, scale));
            if(module==7)  pixels.push_back(pixel(module, channel, 3,  4,  0, 1, 5, gain, relEffic, scale));
            if(module==25) pixels.push_back(pixel(module, channel, 3,  5,  1, 1, 5, gain, relEffic, scale));
            if(module==22) pixels.push_back(pixel(module, channel, 3,  6,  2, 1, 6, gain, relEffic, scale));
            if(module==45) pixels.push_back(pixel(module, channel, 3,  7,  3, 1, 6, gain, relEffic, scale));
            if(module==16) pixels.push_back(pixel(module, channel, 3,  8,  4, 0, 2, gain, relEffic, scale));
            if(module==12) pixels.push_back(pixel(module, channel, 3,  9,  5, 0, 2, gain, relEffic, scale));
            if(module==9)  pixels.push_back(pixel(module, channel, 3, 10,  6, 0, 2, gain, relEffic, scale));
            if(module==31) pixels.push_back(pixel(module, channel, 3, 11,  7, 0, 2, gain, relEffic, scale));
            if(module==15) pixels.push_back(pixel(module, channel, 3, 12,  4, 1, 6, gain, relEffic, scale));
            if(module==11) pixels.push_back(pixel(module, channel, 3, 13,  5, 1, 6, gain, relEffic, scale));
            if(module==30) pixels.push_back(pixel(module, channel, 3, 14,  6, 1, 7, gain, relEffic, scale));
            if(module==1)  pixels.push_back(pixel(module, channel, 3, 15,  7, 1, 7, gain, relEffic, scale));
            if(module==5)  pixels.push_back(pixel(module, channel, 3, 16,  8, 0, 3, gain, relEffic, scale));
            if(module==4)  pixels.push_back(pixel(module, channel, 3, 17,  9, 0, 3, gain, relEffic, scale));
            if(module==3)  pixels.push_back(pixel(module, channel, 3, 18, 10, 0, 3, gain, relEffic, scale));
            if(module==21) pixels.push_back(pixel(module, channel, 3, 19, 11, 0, 3, gain, relEffic, scale));
            if(module==10) pixels.push_back(pixel(module, channel, 3, 20,  8, 1, 7, gain, relEffic, scale));
            if(module==13) pixels.push_back(pixel(module, channel, 3, 21,  9, 1, 7, gain, relEffic, scale));
            if(module==19) pixels.push_back(pixel(module, channel, 3, 22, 10, 1, 8, gain, relEffic, scale));
            if(module==20) pixels.push_back(pixel(module, channel, 3, 23, 11, 1, 8, gain, relEffic, scale));
            if(module==29) pixels.push_back(pixel(module, channel, 4,  0, 12, 0, 4, gain, relEffic, scale));
            if(module==40) pixels.push_back(pixel(module, channel, 4,  1, 13, 0, 4, gain, relEffic, scale));
            if(module==42) pixels.push_back(pixel(module, channel, 4,  2, 14, 0, 4, gain, relEffic, scale));
            if(module==47) pixels.push_back(pixel(module, channel, 4,  3, 15, 0, 4, gain, relEffic, scale));
            if(module==23) pixels.push_back(pixel(module, channel, 4,  4, 12, 1, 8, gain, relEffic, scale));
            if(module==39) pixels.push_back(pixel(module, channel, 4,  5, 13, 1, 8, gain, relEffic, scale));
            if(module==41) pixels.push_back(pixel(module, channel, 4,  6, 14, 1, 9, gain, relEffic, scale));
            if(module==46) pixels.push_back(pixel(module, channel, 4,  7, 15, 1, 9, gain, relEffic, scale));
            if(module==51) pixels.push_back(pixel(module, channel, 4,  8, 16, 0, 5, gain, relEffic, scale));
            if(module==53) pixels.push_back(pixel(module, channel, 4,  9, 17, 0, 5, gain, relEffic, scale));
            if(module==49) pixels.push_back(pixel(module, channel, 4, 10, 16, 1, 9, gain, relEffic, scale));
            if(module==52) pixels.push_back(pixel(module, channel, 4, 11, 17, 1, 9, gain, relEffic, scale));
            
            // South OB
            if(module==27) pixels.push_back(pixel(module, channel, 4, 12,  0, 0, 1, gain, relEffic, scale));
            if(module==32) pixels.push_back(pixel(module, channel, 4, 13,  1, 0, 1, gain, relEffic, scale));
            if(module==34) pixels.push_back(pixel(module, channel, 4, 14,  2, 0, 1, gain, relEffic, scale));
            if(module==36) pixels.push_back(pixel(module, channel, 4, 15,  3, 0, 1, gain, relEffic, scale));
            if(module==28) pixels.push_back(pixel(module, channel, 4, 16,  0, 1, 5, gain, relEffic, scale));
            if(module==33) pixels.push_back(pixel(module, channel, 4, 17,  1, 1, 5, gain, relEffic, scale));
            if(module==35) pixels.push_back(pixel(module, channel, 4, 18,  2, 1, 6, gain, relEffic, scale));
            if(module==37) pixels.push_back(pixel(module, channel, 4, 19,  3, 1, 6, gain, relEffic, scale));
            if(module==38) pixels.push_back(pixel(module, channel, 4, 20,  4, 0, 2, gain, relEffic, scale));
            if(module==50) pixels.push_back(pixel(module, channel, 4, 21,  5, 0, 2, gain, relEffic, scale));
            if(module==55) pixels.push_back(pixel(module, channel, 4, 22,  6, 0, 2, gain, relEffic, scale));
            if(module==57) pixels.push_back(pixel(module, channel, 4, 23,  7, 0, 2, gain, relEffic, scale));
            if(module==44) pixels.push_back(pixel(module, channel, 5,  0,  4, 1, 6, gain, relEffic, scale));
            if(module==54) pixels.push_back(pixel(module, channel, 5,  1,  5, 1, 6, gain, relEffic, scale));
            if(module==56) pixels.push_back(pixel(module, channel, 5,  2,  6, 1, 7, gain, relEffic, scale));
            if(module==58) pixels.push_back(pixel(module, channel, 5,  3,  7, 1, 7, gain, relEffic, scale));
            if(module==60) pixels.push_back(pixel(module, channel, 5,  4,  8, 0, 3, gain, relEffic, scale));
            if(module==63) pixels.push_back(pixel(module, channel, 5,  5,  9, 0, 3, gain, relEffic, scale));
            if(module==8)  pixels.push_back(pixel(module, channel, 5,  6, 10, 0, 3, gain, relEffic, scale));
            if(module==64) pixels.push_back(pixel(module, channel, 5,  7, 11, 0, 3, gain, relEffic, scale));
            if(module==59) pixels.push_back(pixel(module, channel, 5,  8,  8, 1, 7, gain, relEffic, scale));
            if(module==61) pixels.push_back(pixel(module, channel, 5,  9,  9, 1, 7, gain, relEffic, scale));
            if(module==62) pixels.push_back(pixel(module, channel, 5, 10, 10, 1, 8, gain, relEffic, scale));
            if(module==65) pixels.push_back(pixel(module, channel, 5, 11, 11, 1, 8, gain, relEffic, scale));
            if(module==66) pixels.push_back(pixel(module, channel, 5, 12, 12, 0, 4, gain, relEffic, scale));
            if(module==68) pixels.push_back(pixel(module, channel, 5, 13, 13, 0, 4, gain, relEffic, scale));
            if(module==70) pixels.push_back(pixel(module, channel, 5, 14, 14, 0, 4, gain, relEffic, scale));
            if(module==73) pixels.push_back(pixel(module, channel, 5, 15, 15, 0, 4, gain, relEffic, scale));
            if(module==67) pixels.push_back(pixel(module, channel, 5, 16, 12, 1, 8, gain, relEffic, scale));
            if(module==69) pixels.push_back(pixel(module, channel, 5, 17, 13, 1, 8, gain, relEffic, scale));
            if(module==72) pixels.push_back(pixel(module, channel, 5, 18, 14, 1, 9, gain, relEffic, scale));
            if(module==74) pixels.push_back(pixel(module, channel, 5, 19, 15, 1, 9, gain, relEffic, scale));
            if(module==75) pixels.push_back(pixel(module, channel, 5, 20, 16, 0, 5, gain, relEffic, scale));
            if(module==78) pixels.push_back(pixel(module, channel, 5, 21, 17, 0, 5, gain, relEffic, scale));
            if(module==76) pixels.push_back(pixel(module, channel, 5, 22, 16, 1, 9, gain, relEffic, scale));
            if(module==79) pixels.push_back(pixel(module, channel, 5, 23, 17, 1, 9, gain, relEffic, scale));
        }
        
        inFile.close();
        if(pixelSizePrevious == pixels.size()) {
            //cout<<"Not using module "<<imod+1<<endl;
            unusedModules.push_back(imod+1);
        }
        pixelSizePrevious = pixels.size();
    }
    
    for(int i=0; i<unusedModules.size(); i++) {
        cout<<"Not using module "<<unusedModules[i]<<endl;
    }
    
    cc->cd(1);
    hScale->Draw();
    cc->cd(2);
    hEffic->Draw();
    cc->cd(3);
    hPedestalMean->Draw();
    cc->cd(4);
    hPedestalWidth->Draw();
    //cc->Print("diagnostic1D.png");

    // check size of pixel vector
    //cout<<pixels.size()<<endl;
    //return;
    
    // conversion of FPGA channel to physical coordiante
    int fullrows[8][48] = {{35, 33, 34, 32, 28, 30, 29, 31, 99, 97, 98, 96, 92, 94, 93, 95, 163, 161, 162, 160, 156, 158, 157, 159, 131, 129, 130, 128, 188, 190, 189, 191, 67, 65, 66, 64, 124, 126, 125, 127, 3, 1, 2, 0, 60, 62, 61, 63}, {39, 37, 38, 36, 24, 26, 25, 27, 103, 101, 102, 100, 88, 90, 89, 91, 167, 165, 166, 164, 152, 154, 153, 155, 135, 133, 134, 132, 184, 186, 185, 187, 71, 69, 70, 68, 120, 122, 121, 123, 7, 5, 6, 4, 56, 58, 57, 59}, {43, 41, 42, 40, 20, 22, 21, 23, 107, 105, 106, 104, 84, 86, 85, 87, 171, 169, 170, 168, 148, 150, 149, 151, 139, 137, 138, 136, 180, 182, 181, 183, 75, 73, 74, 72, 116, 118, 117, 119, 11, 9, 10, 8, 52, 54, 53, 55}, {47, 45, 46, 44, 16, 18, 17, 19, 111, 109, 110, 108, 80, 82, 81, 83, 175, 173, 174, 172, 144, 146, 145, 147, 143, 141, 142, 140, 176, 178, 177, 179, 79, 77, 78, 76, 112, 114, 113, 115, 15, 13, 14, 12, 48, 50, 49, 51}, {51, 49, 50, 48, 12, 14, 13, 15, 115, 113, 114, 112, 76, 78, 77, 79, 179, 177, 178, 176, 140, 142, 141, 143, 147, 145, 146, 144, 172, 174, 173, 175, 83, 81, 82, 80, 108, 110, 109, 111, 19, 17, 18, 16, 44, 46, 45, 47}, {55, 53, 54, 52, 8, 10, 9, 11, 119, 117, 118, 116, 72, 74, 73, 75, 183, 181, 182, 180, 136, 138, 137, 139, 151, 149, 150, 148, 168, 170, 169, 171, 87, 85, 86, 84, 104, 106, 105, 107, 23, 21, 22, 20, 40, 42, 41, 43}, {59, 57, 58, 56, 4, 6, 5, 7, 123, 121, 122, 120, 68, 70, 69, 71, 187, 185, 186, 184, 132, 134, 133, 135, 155, 153, 154, 152, 164, 166, 165, 167, 91, 89, 90, 88, 100, 102, 101, 103, 27, 25, 26, 24, 36, 38, 37, 39}, {63, 61, 62, 60, 0, 2, 1, 3, 127, 125, 126, 124, 64, 66, 65, 67, 191, 189, 190, 188, 128, 130, 129, 131, 159, 157, 158, 156, 160, 162, 161, 163, 95, 93, 94, 92, 96, 98, 97, 99, 31, 29, 30, 28, 32, 34, 33, 35}};

    // load equalized gain from Andrew's TOT analysis
    ifstream equalizeTOTfile;
    equalizeTOTfile.open("dT_Equalized_Gains.txt");
    string myChannel[6912], equalizeTOTgain[2][6912];
    if(equalizeTOTfile.is_open()) {
        for(int i = 0; i < 6912*3; ++i){
            equalizeTOTfile >> myChannel[i];
            equalizeTOTfile >> equalizeTOTgain[1][i]; // South OB
            equalizeTOTfile >> equalizeTOTgain[0][i]; // North OB
        }
    }
    
    //TFile *f = TFile::Open("../ThresholdScan/Feb5_Run060505/scanHist.root");
    //TFile *f = TFile::Open("../ThresholdScan/Feb10_Run060602/scanHist.root");
    //TFile *f = TFile::Open("../ThresholdScan/Feb14_Run060732/scanHist.root");
    //TFile *f = TFile::Open("../ThresholdScan/Feb19_Run060866/scanHist.root");
    TFile *f = TFile::Open(Form("ThresholdScan/scanHist_%d.root", run));
    //f->ls();
    
    // loop to fill gain, efficiency, and pedestal tables
    double pedestal[72][3] {0};
    double effic[6912*2] {0};
    for(int ipix=0; ipix<pixels.size(); ipix++) {
        
        pixel locPixel = pixels.at(ipix);
        
        int modrow = locPixel.modrow;
        int modcol = locPixel.modcol;
        //if(modcol > 0) {
        int chan = locPixel.chan;
        int pixel = 0;
        for(int i=0; i<8; i++) {
            for(int j=0; j<48; j++) {
                if(fullrows[i][j] == chan && ((j<24 && modcol<1) || (j>23 && modcol>0)) ) {
                    // found correct channel number, now fill histogram with maps
                    int pixelrow = modrow*8 + i;
                    int pixelcol = modcol*3 + j%8;
                    int pmtrow = pixelrow/8;
                    int pmtcol = j/8;
                    int globpixel = (pmtrow*64 + pmtcol*64*18) + i + (j - 8*pmtcol)*8;
                    
                    int ibox = 1; // South OB default
                    if(locPixel.slot == 3 || (locPixel.slot == 4 && locPixel.fiber < 12)) ibox = 0; // slot/fibers for North OB
                    
                    if(equalizeTOT) {
                        cout<<"equalize TOT "<<globpixel<<" gain="<<locPixel.gain<<endl;
                        pixels.at(ipix).gain = atoi(equalizeTOTgain[ibox][globpixel].data());
                        locPixel = pixels.at(ipix);
                        cout<<"new gain="<<locPixel.gain<<endl;
                    }
                    
                    //cout<<chan<<" "<<pmtrow<<" "<<pixelrow<<" "<<pmtcol<<" "<<pixelcol<<" "<<globpixel<<" "<<locPixel.gain<<" "<<locPixel.effic<<endl;
                    hGlobalPixelMap[ibox]->Fill(pixelrow, j, globpixel);
                    hGainMap[ibox]->Fill(pixelrow, j, locPixel.scale);
                    hEfficMap[ibox]->Fill(pixelrow, j, locPixel.effic);
                    hSSPslotMap[ibox]->Fill(pixelrow, j, locPixel.slot);
                    hSSPfiberMap[ibox]->Fill(pixelrow, j, locPixel.fiber);
                    hFiberDAQmonMap[ibox]->Fill(pixelrow, j, (locPixel.slot-3)*24 + locPixel.fiber);
                    hLVGroupMap[ibox]->Fill(pixelrow, j, locPixel.lv);
                    hLaserModuleMap[ibox]->Fill(pixelrow, j, locPixel.mod);
                    hEqualizedGain[ibox]->SetBinContent(globpixel+1, locPixel.gain);
                    
                    // set efficiency for CCDB
                    effic[globpixel + ibox*6912] = locPixel.effic;
                    
                    int DAQmonFiber = (locPixel.slot-3)*24 + locPixel.fiber; // - 36.; (need to subtract 36 for Spring 2019 threshold scans)
                    int ASICnum = j/8;
                    if(j > 23) ASICnum = abs(ASICnum-5);
                    TH2F *hScan2D = (TH2F*)f->Get(Form("hScan_%d_%d",DAQmonFiber,ASICnum));
                    if(!hScan2D) continue;
                    TH1F *hScan = (TH1F*)hScan2D->ProjectionY("test",chan%64+1,chan%64+1);
                    hScan->GetXaxis()->SetRangeUser(160, 220); // zoom in on expected range
                    double pedMean = hScan->GetMean(); // 0;
                    double pedSigma = hScan->GetRMS(); //5;
                    hScan->GetXaxis()->SetRangeUser(pedMean-10, pedMean+10); // zoom into fixed range around pedestal
                    pedMean = hScan->GetMean();
                    pedSigma = hScan->GetRMS();
                    
                    pedestal[DAQmonFiber][ASICnum] += pedMean;
                    
                    /*
                    double maxRate = 0;
                    for(int k=0; k<hScan->GetNbinsX(); k++) {
                        double threshold = hScan->GetBinCenter(k+1);
                        if(hScan->GetBinContent(k+1) > maxRate) {
                            maxRate = hScan->GetBinContent(k+1);
                            pedMean = hScan->GetBinCenter(k+1);
                        }
                    }
                    TF1 *fgaus = new TF1("fgaus", "gaus", 150, 250);
                    fgaus->SetParameter(1,pedMean); fgaus->SetParameter(2,pedSigma);
                    hScan->Fit(fgaus,"Q","",pedMean-pedSigma,pedMean+pedSigma);
                    pedMean = fgaus->GetParameter(1);
                    pedSigma = fgaus->GetParameter(2);
                    */
                    
                    // set dummy pixels to 0
                    if(globpixel < 11*64 || (globpixel >= 5*18*64 &&  globpixel < 5*18*64+7*64)) {
                        pedMean = 0;
                        pedSigma = 0;
                    }
                    
                    hPedestalMeanMap[ibox]->Fill(pixelrow, j, pedMean);
                    hPedestalSigmaMap[ibox]->Fill(pixelrow, j, pedSigma);
                }
            }
        }
    }
    
    // loop to write effic CCDB files
    ofstream efficfile;
    efficfile.open("effic_fall2019_south.txt");
    for(int i=0; i<6912; i++)
        efficfile<<effic[i]<<endl;
    efficfile.close();
    
    efficfile.open("effic_fall2019_north.txt");
    for(int i=0; i<6912; i++)
        efficfile<<effic[i+6912]<<endl;
    efficfile.close();
    
    cout<<"fill MAROC calib file (will take a few minutes...)"<<endl; 
        
    // loop to fill MAROC calibration file
    ofstream configfile;
    bool fixedThreshold = false;
    TString gainString = "equalizeVer1";
    if(minGain) gainString = "equalizeVer2";
    if(equalizeTOT) gainString = "equalizeTotVer2";
    if(unityGain) gainString = "unityGain";
    configfile.open(Form("config/rocdirc_ssp_%s_DAC%d_Pedestal%d.cnf", gainString.Data(), DAC, run));
    int locDACthreshold = DAC;
    for(int islot=3; islot<6; islot++) {
        
        configfile<<"########################"<<endl;
        configfile<<"SSP_SLOTS   "<<islot<<endl;
        configfile<<"########################"<<endl<<endl;
        
        for(int ifiber=0; ifiber<24; ifiber++) {
            //if(islot==4 && ifiber<12) continue;  // for South only
            
            int ibox = 1;
            if(islot == 3 || (islot == 4 && ifiber < 12)) ibox = 0;
            
            configfile<<"########################"<<endl;
            configfile<<"SSP_FIBER   "<<ifiber<<endl;
            configfile<<"########################"<<endl<<endl;
            
            for(int iasic=0; iasic<3; iasic++) {
                int asicDivider = 64;
                
                configfile<<"SSP_ASIC   "<<iasic<<endl;
                int locFiber = (islot-3)*24 + ifiber; // -36.; (need to subtract 36 for Spring 2019 threshold scans)
                //cout<<ibox<<" "<<islot<<" "<<ifiber<<" "<<locFiber<<" "<<pedestal[ifiber][iasic]<<endl;
                if(fixedThreshold) configfile<<"SSP_MAROC_REG_DAC0   "<<400<<endl;
                else configfile<<"SSP_MAROC_REG_DAC0   "<<(int)(pedestal[locFiber][iasic]/64.)+locDACthreshold<<endl;
                
                // get all gains for this asic
                configfile<<"SSP_MAROC_REG_GAIN_0_15      ";
                for(int ichan=0; ichan<16; ichan++) {
                    for(int ipix=0; ipix<pixels.size(); ipix++) {
                        pixel locPixel = pixels.at(ipix);
                        if(ichan==0) {
                            //cout<<"chan = "<<ichan<<" "<<locPixel.chan%asicDivider<<endl;
                            //cout<<"asic = "<<iasic<<" "<<locPixel.chan/asicDivider<<endl;
                        }
                        if(ichan == locPixel.chan%64 && iasic == locPixel.chan/64 && islot == locPixel.slot && ifiber == locPixel.fiber) {
                            if(unityGain && locPixel.gain != 0) configfile<<64<<" ";
                            else configfile<<locPixel.gain<<" ";
                        }
                    }
                }
                configfile<<endl;
                configfile<<"SSP_MAROC_REG_GAIN_16_31     ";
                for(int ichan=16; ichan<32; ichan++) {
                    for(int ipix=0; ipix<pixels.size(); ipix++) {
                        pixel locPixel = pixels.at(ipix);
                        if(ichan == locPixel.chan%asicDivider && iasic == locPixel.chan/64 && islot == locPixel.slot && ifiber == locPixel.fiber) {
                            if(unityGain && locPixel.gain != 0) configfile<<64<<" ";
                            else configfile<<locPixel.gain<<" ";
                        }
                    }
                }
                configfile<<endl;
                configfile<<"SSP_MAROC_REG_GAIN_32_47     ";
                for(int ichan=32; ichan<48; ichan++) {
                    for(int ipix=0; ipix<pixels.size(); ipix++) {
                        pixel locPixel = pixels.at(ipix);
                        if(ichan == locPixel.chan%asicDivider && iasic == locPixel.chan/64 && islot == locPixel.slot && ifiber == locPixel.fiber) {
                            if(unityGain && locPixel.gain != 0) configfile<<64<<" ";
                            else configfile<<locPixel.gain<<" ";
                        }
                    }
                }
                configfile<<endl;
                configfile<<"SSP_MAROC_REG_GAIN_48_63     ";
                for(int ichan=48; ichan<64; ichan++) {
                    for(int ipix=0; ipix<pixels.size(); ipix++) {
                        pixel locPixel = pixels.at(ipix);
                        if(ichan == locPixel.chan%asicDivider && iasic == locPixel.chan/64 && islot == locPixel.slot && ifiber == locPixel.fiber) {
                            if(unityGain && locPixel.gain != 0) configfile<<64<<" ";
                            else configfile<<locPixel.gain<<" ";
                        }
                    }
                }
                configfile<<endl;
            }
            configfile<<endl;
        }
    }
    configfile.close();
    
    TCanvas *dd = new TCanvas("dd", "dd", 1600, 600);
    gStyle->SetOptStat(0);
    
    TString box = "N";
    TFile *fout = new TFile("maps.root","recreate");
    TText *tx = new TLatex(-20, -8, "North OB");
    tx->SetTextSize(0.09);
    for(int ibox=0; ibox<2; ibox++) {
        if(ibox == 1) {
            box = "S";
            tx->SetText(-20, -8, "South OB");
        }
        hGainMap[ibox]->SetMinimum(0);
        hGainMap[ibox]->SetMaximum(500);
        hGainMap[ibox]->Draw("colz");
        tx->Draw("same");
        dd->Print(Form("maps/gainMap%s.pdf", box.Data()));
        
        hEfficMap[ibox]->SetMaximum(1.4);
        hEfficMap[ibox]->SetMinimum(0.6);
        hEfficMap[ibox]->Draw("colz");
        tx->Draw("same");
        dd->Print(Form("maps/efficMap%s.pdf", box.Data()));
        
        hSSPslotMap[ibox]->Rebin2D(8,8*3);
        hSSPslotMap[ibox]->Scale(1/64./3.);
        hSSPslotMap[ibox]->SetMarkerSize(3);
        hSSPslotMap[ibox]->Draw("colz text");
        tx->Draw("same");
        dd->Print(Form("maps/sspSlotMap%s.png", box.Data()));
    
        hSSPfiberMap[ibox]->Rebin2D(8,8*3);
        hSSPfiberMap[ibox]->Scale(1/64./3.);
        hSSPfiberMap[ibox]->SetMarkerSize(3);
        hSSPfiberMap[ibox]->Draw("colz text");
        tx->Draw("same");
        dd->Print(Form("maps/sspFiberMap%s.png", box.Data()));
        
        hFiberDAQmonMap[ibox]->Rebin2D(8,8*3);
        hFiberDAQmonMap[ibox]->Scale(1/64./3.);
        hFiberDAQmonMap[ibox]->SetMarkerSize(3);
        hFiberDAQmonMap[ibox]->Draw("colz text");
        tx->Draw("same");
        dd->Print(Form("maps/DAQmonFiberMap%s.png", box.Data()));
        
        hLVGroupMap[ibox]->Rebin2D(8,8*3);
        hLVGroupMap[ibox]->Scale(1/64./3.);
        hLVGroupMap[ibox]->SetMarkerSize(3);
        hLVGroupMap[ibox]->Draw("colz text");
        tx->Draw("same");
        dd->Print(Form("maps/lvGroupMap%s.png", box.Data()));
        
        hLaserModuleMap[ibox]->Rebin2D(8,8*3);
        hLaserModuleMap[ibox]->Scale(1/64./3.);
        hLaserModuleMap[ibox]->SetMarkerSize(3);
        hLaserModuleMap[ibox]->Draw("colz text");
        tx->Draw("same");
        dd->Print(Form("maps/laserModuleMap%s.png", box.Data()));
        
        hPedestalMeanMap[ibox]->SetMinimum(160);
        hPedestalMeanMap[ibox]->SetMaximum(210);
        hPedestalMeanMap[ibox]->Draw("colz");
        tx->Draw("same");
        dd->Print(Form("maps/pedMeanMap%s.png", box.Data()));
        
        hPedestalSigmaMap[ibox]->SetMinimum(0);
        hPedestalSigmaMap[ibox]->SetMaximum(10);
        hPedestalSigmaMap[ibox]->Draw("colz");
        tx->Draw("same");
        dd->Print(Form("maps/pedSigmaMap%s.png", box.Data()));
        
        //hGlobalPixelMap->Draw("htext");
        //dd->Print("maps/globalPixelMap.pdf");
        
        hGainMap[ibox]->Write();
        hEfficMap[ibox]->Write();
        hPedestalMeanMap[ibox]->Write();
        hEqualizedGain[ibox]->Write();
    }
    fout->Close();
    
    return;
}
