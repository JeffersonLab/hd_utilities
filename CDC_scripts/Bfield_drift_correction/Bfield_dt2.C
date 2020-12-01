{

  // plot Garfield drift time tables for different magnetic field strengths
  // calculate time difference w r t 1.30T 
  // fit curves to time differences


  //  gROOT->Reset();
  gROOT->SetStyle("Plain");
  gStyle->SetPalette(1);
  gStyle->SetPadLeftMargin(0.12);

  gStyle->SetFillStyle(4000);
  gStyle->SetFillColor(0);
  //gStyle->SetFrameFillStyle(4000);


  gStyle->SetTitleOffset(1.2,"X");
  gStyle->SetTitleOffset(1.7,"Y");
  gStyle->SetLabelSize(0.03,"XY");
  gStyle->SetTitleSize(0.03,"XY");


  #define COL1 907;
  #define COL2 860;
  #define COL3 839;
  const Int_t col[6]={628,801,820,860,840,880};

  
  Char_t filename[500];
  Char_t wholeline[200];

  Char_t title1[250];

  const Int_t nt=78; // number of times in each file
  //really 77, ignore last one

  FILE *file;

  Int_t i,j;

  Float_t r,d;

  const Int_t nb=14; // number of B field values

  Double_t b[nb];

  Int_t colour[nb];

  Int_t bcount =0;

  //  Int_t bmin=25; //start with 1.25T
  Int_t bmin=30; //start with 1.30T
  Int_t bmax=95; //stop with 1.95T


  for (i=bmin;i<=bmax; i+=5) {

    b[bcount] = 1 + 0.01*i;

    colour[bcount] = col[bcount%6];

    bcount++;

  }

  colour[0] = 907;  //use a bright color at one end to identify the start

  cout << bcount << endl;

  //  return;


  //  Double_t b[nb] = {1.3,1.4,1.5,1.6,1.7,1.8,1.9};//,1.6,1.7,1.8,1.9}; //bfield values in T
  //Double_t b[nb] = {1.3,1.4,1.6,1.7,1.8,1.9};//,1.6,1.7,1.8,1.9}; //bfield values in T

  Int_t fiti; //fit graph g[fiti] with mag field b[fiti]

  Double_t radius[nt]; 

  Double_t dtime[nb][nt] = {0};
  Double_t tdiff[nb][nt] = {0};



  Int_t idefault = 0; //default B is in 0th place in b array (1.3T)
  Double_t bdefault = b[idefault];

  // read in data files

  for (i=0; i<nb; i++) {

    //printf("B %f B*100 %5f (B-1)*100 %5f Int((B-1)*100) %2.0f \n",b[i],b[i]*100,(b[i]-1.0)*100,(b[i]-1.0)*100);
    //     sprintf(filename,"xt-lookup/bigcdc/xt_cdc_8mm/xt_cdc_8mm_2100-1_%2.0fT-295K-5000-5000-20um.asc",(b[i]-1.0)*100);
    //    sprintf(filename,"/raid1/cdc/garfield/xt-lookup/bigcdc/xt_cdc_8mm_296K_e32_lines100/xt_cdc_8mm_2100-1_%2.0fT-296K-5000-5000-20um.asc",(b[i]-1.0)*100);

    ///raid1/cdc/garfield/xt_halld/isopropanol/xt_cdc_8mm_2125-1_30T-296K-760mmHg-4950-4950-propanol-100-20um.asc

    sprintf(filename,"/raid1/cdc/garfield/xt_halld/isopropanol/xt_cdc_8mm_2125-1_%2.0fT-296K-760mmHg-4950-4950-propanol-100-20um.asc",(b[i]-1.0)*100);

    if ((file=fopen(filename,"r")) == NULL) {
      printf("Garfield look up table %s does not exist\n",filename);
      gApplication->Terminate();
    }

    cout << "Opened file " << filename << "\n";

    //skip header part of file.  (these lines start with #)
    for (j=0; j<21; j++) {
      fgets(wholeline,200,file);
      //  printf("%s",wholeline);  //the char array includes the \n
    }

  
    for (j=0; j<nt; j++) {

      fscanf(file,"%f %f\n",&r,&d);   //radius (cm), drift time (us).  must be floats, not double. 

      //printf("%i %f %f\n",j,r,d);   //radius (cm), drift time (us)

      dtime[i][j]=(Double_t)1000*d; //convert to ns   
  
      if (i==0) radius[j]=(Double_t)(10*r); //convert to mm

      if (radius[j] != 10*r) printf("problem: radius # %i for %f T is %f, for %f T was %f\n",j,b[i],10*r,b[0],radius[j]);

    }  

    fclose(file);
  }

  cout << endl;

  // calc time diff between files

  //  for (i=0; i<nb; i++) {
  for (i=nb-1; i>=0; i--) {  // start with 1.9 and work down to 1.4T

    for (j=0; j<nt; j++) {

      tdiff[i][j] = dtime[i][j] - dtime[idefault][j];






    }  

  }



  TMultiGraph *mg1 = new TMultiGraph;
  mg1->SetTitle("Garfield calcs for range of B field, 49.5/49.5/1 Ar/Co2/isopropanol, 2125V; Drift radius (mm); Drift time (ns)"); 

  TLegend* leg1 = new TLegend(0.2,0.4,0.3,0.85);
  leg1->SetFillStyle(0);
  leg1->SetFillColor(0);
  leg1->SetBorderSize(0);


  TGraph *g1[nb];

  //  for (i=0; i<nb; i++) {
  for (i=nb-1; i>=0; i--) {

      g1[i] = new TGraph(nt,radius,&dtime[i][0]);

      g1[i]->SetMarkerStyle(21);
      g1[i]->SetMarkerSize(0.2);
      g1[i]->SetMarkerColor(colour[i]);
      g1[i]->SetLineColor(colour[i]);

      mg1->Add(g1[i]);
      leg1->AddEntry(g1[i],Form("%4.2fT",b[i]),"L");

  }



  mg1->Draw("ACP");

  leg1->Draw();


  


  new TCanvas;

  TMultiGraph *mg = new TMultiGraph;
  mg->SetTitle(Form("Time difference in Garfield calcs between B fields and %4.2fT, 49.5/49.5/1 Ar/Co2/isopropanol, 2125V; Drift time for higher field (ns); Difference in drift time (ns)",bdefault)); 

  TLegend* leg = new TLegend(0.2,0.4,0.3,0.85);
  leg->SetFillStyle(4000);
  leg->SetFillColor(0);
  leg->SetBorderSize(0);


  //spot checks
  //  printf("%f %f %f %f\n",radius[10],dtime[1][10],dtime[idefault][10],tdiff[1][10]);

  TGraph *g[nb];

  for (i=nb-1; i>=0; i--) {
    //  for (i=0; i<nb; i++) {

    if (i!=idefault) {

      g[i] = new TGraph(nt,&dtime[i][0],&tdiff[i][0]);

      g[i]->SetMarkerStyle(21);
      g[i]->SetMarkerSize(0.2);
      g[i]->SetMarkerColor(colour[i]);
      g[i]->SetLineColor(colour[i]);

      mg->Add(g[i]);
      leg->AddEntry(g[i],Form("%4.2fT",b[i]),"L");

    }

  }



  mg->Draw("ACP");

  leg->Draw();



  
  TF1 *f = new TF1("f","[0]*x",0,700);  


  f->SetParameter(0,0);


  f->SetLineWidth(1);
  f->SetLineColor(rand()%100 + 800);


  Double_t param0[nb-1];
  Double_t param1[nb-1];
  Double_t fitb[nb-1];


  Double_t fpars[6];
  Double_t maxerr=0;
  Double_t averr=0;
  Double_t fit,err,x;
  Int_t maxerrj;
  Int_t maxj;

  double fitstatus;

  Int_t ntofit=0;  //collect fit params into array to fit each param, but exclude 1.5T results

  TPaveText *t[nb];


  for (fiti=1; fiti<nb; fiti++) {

    new TCanvas; 

    g[fiti]->SetTitle(Form("Difference in predicted drift time for %4.2fT and %4.2fT;Drift time at %4.2fT (ns); Drift time at %4.2fT - drift time at %4.2fT (ns)",b[fiti],bdefault,b[fiti],b[fiti],bdefault));

    g[fiti]->Draw("ACP");

    fitstatus = g[fiti]->Fit("f","Q");


   // calc max and ave abs error for this fit

    maxerr=0;
    averr=0;

    maxj =nt;  // use all the points 

    f->GetParameters(fpars);


  
    t[fiti] = new TPaveText(0.16,0.7,0.56,0.99,"NDC");

    t[fiti]->AddText(Form("Fit function: %5.3f*drift-time",fpars[0]));
    t[fiti]->SetBorderSize(0);
    t[fiti]->SetTextAlign(12);
    t[fiti]->SetFillStyle(0);

    t[fiti]->Draw("");





    for (j=0; j<maxj; j++) {

      // printf("j %i radius(j) %f\n",j,radius[j]);

      x = dtime[fiti][j];

      fit = fpars[0]*x;

      err = tdiff[fiti][j] - fit;

      averr += fabs(err);

      //  cout << tdiff[fiti][j] << "\t" << fit << "\t" << err<< "\t" << maxerr << "\t" << fabs(err)<< "\t" << fabs(maxerr) << endl;

      if ( fabs(maxerr) < fabs(err) ) {
        maxerr= err;
        maxerrj= j;
      }

    }  

    averr = averr/(Double_t)maxj;

    printf("%4.2fT fit status %.0f param0 %5.3f av abs err %3.1fns max abs err %3.1fns at radius %3.1fmm \n",b[fiti],fitstatus,fpars[0],averr,maxerr,radius[maxerrj]);


    // exclude 1.5T fit params from global fits
    if (b[fiti] != 10.5) {   

      param0[ntofit] = fpars[0];
      //      param1[ntofit] = fpars[1];
      fitb[ntofit] = b[fiti]; 
      //      printf("including param0 %4.2f and param1 %4.2f for %4.2fT in global fit as # %i\n",param0[ntofit],param1[ntofit],fitb[ntofit],ntofit);
      ntofit++;
    }


  }
  
  

  new TCanvas;

  TGraph *gpar0 = new TGraph(ntofit,fitb,param0);

  gpar0->SetMarkerStyle(21);
  gpar0->SetMarkerSize(0.2);
  gpar0->SetMarkerColor(877);
  gpar0->SetLineColor(877);

  gpar0->Draw("ALP");

  gpar0->Fit("pol1","Q");
  gpar0->GetFunction("pol1")->SetLineWidth(1);
  gpar0->GetFunction("pol1")->SetLineColor(38);

  Double_t gfitpars[4];

  gpar0->GetFunction("pol1")->GetParameters(gfitpars);

  Char_t fitfun0[200];

  //  sprintf(fitfun0,"%4.2f + %4.2fx + %4.2fx^2 + %4.2fx^3",gfitpars[0],gfitpars[1],gfitpars[2],gfitpars[3]);
  sprintf(fitfun0,"%5.3f + %5.3fx",gfitpars[0],gfitpars[1]);

  printf("\nGlobal fit param 0 fit status %.0f function %s\n",fitstatus,fitfun0);

  gpar0->SetTitle(Form("Fit parameter k_{B}; B field (T); k_{B}"));


  TPaveText *gtxt = new TPaveText(0.15,0.75,0.55,0.85,"NDC");

  gtxt->AddText(Form("Fit function: %5.3f + %5.3f*B",gfitpars[0],gfitpars[1]));
  //  txt->AddText(Form("[0] = %4.2f + %4.2fB + %4.2fB^2 + %4.2fB^3",gfitpars[0],gfitpars[1],gfitpars[2],gfitpars[3]));
  gtxt->SetBorderSize(0);
  gtxt->SetTextAlign(12);
  gtxt->SetFillStyle(0);

  gtxt->Draw("");



  //revisit datasets and calc diff betw data and global fit



  Double_t gblfit0,gblfit1;


  for (i=1; i<nb; i++) {

   // calc max and ave abs error for this fit


    maxerr=0;
    averr=0;

    maxj =nt;  // use all the points 

    //calc fit params

    gblfit0 = gfitpars[0] + gfitpars[1]*b[i];

    //   gblfit0 = -0.08 + 0.07*b[i];  //*****************************rounded*******************************


    for (j=0; j<maxj; j++) {

      //printf("j %i radius(j) %f\n",j,radius[j]);

      x = dtime[i][j];

      fit = gblfit0*x;

      err = tdiff[i][j] - fit;

      averr += fabs(err);

     //     cout << tdiff[4][j] << "\t" << fit << "\t" << err<< "\t" << maxerr << "\t" << fabs(err)<< "\t" << fabs(maxerr) << endl;

      if ( fabs(maxerr) < fabs(err) ) {
        maxerr= err;
        maxerrj= j;
      }

    }  

    averr = averr/(Double_t)maxj;

    printf("%4.2fT Global fit av abs err %3.1fns max abs err %3.1fns at radius %3.1fmm \n",b[i],averr,maxerr,radius[maxerrj]);
  

  }

  new TCanvas;

  // draw data with global fit

  TMultiGraph *mgfit = new TMultiGraph;

  mgfit->SetTitle(Form("Difference from drift time at %4.2fT with fitted function;Drift time (ns); Drift time - drift time at %4.2fT (ns)",bdefault,bdefault));
  TLegend* legfit = new TLegend(0.2,0.6,0.3,0.85);
  legfit->SetFillStyle(4000);
  legfit->SetFillColor(0);
  legfit->SetBorderSize(0);
  
  TF1 *ff[nb];

  TGraph *g2[nb];


  for (i=1; i<nb; i++) {

    g2[i] = new TGraph;

    g2[i] = (TGraph*)g[i]->Clone();

    g2[i]->GetFunction("f")->SetBit(TF1::kNotDraw);   //**** don't draw the individual fits

    mgfit->Add(g2[i]);
    legfit->AddEntry(g2[i],Form("%4.2fT",b[i]),"L");

    gblfit0 = gfitpars[0] + gfitpars[1]*b[i]; 


    ff[i] = new TF1(Form("ff[%i]",i),"[0]*x",0,700);  

    ff[i]->SetParameter(0,gblfit0);
    ff[i]->SetLineColor(38);
    ff[i]->SetLineWidth(1);
    
  }
  

  mgfit->Draw("ACP");
  
  legfit->SetX1(0.15);
  legfit->SetX2(0.3);
  legfit->SetY1(0.4);
  legfit->SetY2(0.85);

  legfit->Draw();


  for (i=1; i<nb; i++) {

    ff[i]->Draw("same");
 
  }
  
  TPaveText *txt = new TPaveText(0.3,0.73,0.68,0.83,"NDC");

  txt->AddText("Fit function: k_{B}*drift-time");
  //  txt->AddText(Form("[0] = %4.2f + %4.2fB + %4.2fB^2 + %4.2fB^3",gfitpars[0],gfitpars[1],gfitpars[2],gfitpars[3]));
  txt->AddText(Form("k_{B} = %5.3f + %5.3fB",gfitpars[0],gfitpars[1]));
  txt->SetBorderSize(0);
  txt->SetTextAlign(12);
  txt->SetFillStyle(0);

  txt->Draw("");


  c1->SaveAs("pngs/t_vs_r.png");
  c1_n2->SaveAs("pngs/tdiff_vs_t.png");

  Char_t cname[6];

  TCanvas *c;

  for (i=1; i<nb; i++) {
    sprintf(filename,"pngs/fit_1_%2.0fT.png",(b[i]-1.0)*100);
    sprintf(cname,"c1_n%i",i+2);

    c = (TCanvas*)gROOT->FindObject(cname);
    c->SaveAs(filename);
  }

  sprintf(cname,"c1_n%i",nb+2);
  c = (TCanvas*)gROOT->FindObject(cname);
  c->SaveAs("pngs/kfit.png");

  sprintf(cname,"c1_n%i",nb+3);
  c = (TCanvas*)gROOT->FindObject(cname);
  c->SaveAs("pngs/dtfit.png");

  
}

