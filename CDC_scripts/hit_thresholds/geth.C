
// read in thresholds from parameter files.

void readchannellist(Int_t roc25n[19][72], Int_t roc26n[19][72], Int_t roc27n[19][72], Int_t roc28n[19][72]) {

  // Plain text file of N, ring, straw, rocid, slot and channel in format %4i %2i %3i %2i %2i %2i
  //CDC_straw_numbers_run_3221.C

  Char_t filename[500];
  Char_t line[200]; 

  sprintf(filename,"CDC_straw_numbers_run_3221.txt");

  FILE *translate = fopen(filename,"r");

  if (!translate) printf("Cannot find %s\n",filename);
  if (!translate) return;

  printf("Reading connector index from %s\n",filename);
  
  fscanf(translate,"%*s %*s %s\n",line);  //  # CDC channels
  fscanf(translate,"%*s %*s %*s %*s %*s %*s %s\n",line);  // # N ring straw roc slot channel

  int n,ring,straw,rocid,slot,chan;

    
  while (!feof(translate)) {

    fscanf(translate,"%i %i %i %i %i %i\n",&n,&ring,&straw,&rocid,&slot,&chan);

    //printf("n %i ring %i straw %i roc %i slot %i chan %i\n", n,ring,straw,rocid,slot,chan);

    if (rocid==25) roc25n[slot][chan] = n;
    if (rocid==26) roc26n[slot][chan] = n;
    if (rocid==27) roc27n[slot][chan] = n;
    if (rocid==28) roc28n[slot][chan] = n;
 
  }
    
  fclose(translate);

  return;

}
 

void readconfigfile(int iroc, Int_t *harray, int thres[3522]) {

  //printf("Run number %06i\n",runnumber);

  Char_t filename[500];
  Char_t line[200]; 

  int i;
  Int_t junk;
  Float_t fjunk;

  Int_t pedestals[19][72];   //pedestal - don't need but keeping 

  int slot;

  Int_t *parray;


  sprintf(filename,"roccdc%i_fadc125_summer20_B.cnf",iroc-24);

    printf("Looking for config parameter file %s\n",filename);

    FILE *config = fopen(filename,"r");
    
    if (!config) printf("Could not open config file for cdcroc%i\n",iroc);
    if (!config) return;

    parray = &pedestals[0][0];
    

    fscanf(config,"\n");    //new line
    fscanf(config,"################\n"); 
    fscanf(config,"CRATE    %s\n",line); 
    printf("%s \n",line);  // print roc name
       
    fscanf(config,"################\n"); 

    fscanf(config,"\n\n"); //2 blank lines

    while (!feof(config)) {

      fscanf(config,"%s\n",line);
      if (!strcmp(line, "################\n")) break;
 
      //   fscanf(config,"FADC125_SLOTS   %i\n",&slot);
      fscanf(config,"%s   %i\n",line,&slot);
      fscanf(config,"################\n"); 

      printf("%s slot %i\n",line,slot);

      fscanf(config,"\n\n"); //2 empty lines


      //fscanf(config,"FADC125_DAC_CH_00_17   ");
      fscanf(config,"%s   ",line);
      //printf("DAC: %s\n",line);
      if (!strcmp(line,"FADC125_DAC_CH_00_17   ")) printf("mismatch\n");
      if (!strcmp(line,"FADC125_DAC_CH_00_17   ")) break;

      for (i=0; i<18; i++) fscanf(config,"  %i",&junk);
      fscanf(config,"\n"); 

      fscanf(config,"FADC125_DAC_CH_18_35   ");
      for (i=0; i<18; i++) fscanf(config,"  %i",&junk);
      fscanf(config,"\n"); 

      fscanf(config,"FADC125_DAC_CH_36_53   ");
      for (i=0; i<18; i++) fscanf(config,"  %i",&junk);
      fscanf(config,"\n"); 

      fscanf(config,"FADC125_DAC_CH_54_71   ");
      for (i=0; i<18; i++) fscanf(config,"  %i",&junk);
      fscanf(config,"\n"); 

      fscanf(config,"\n\n"); //2 empty lines


      //fscanf(config,"FADC125_THR_CH_00_17   ");
      fscanf(config,"%s   ",line);
      //      printf("THR: %s\n",line);
      if (!strcmp(line,"FADC125_THR_CH_00_17   ")) printf("THR mismatch\n");
      // fscanf(config,"FADC125_THR_CH_00_17   ");

      for (i=0; i<18; i++) fscanf(config,"  %i",harray+72*slot+i);
      fscanf(config,"\n"); 
 
 
      fscanf(config,"FADC125_THR_CH_18_35   ");
      for (i=18; i<36; i++) fscanf(config,"  %i",harray+72*slot+i);
      fscanf(config,"\n"); 

      fscanf(config,"FADC125_THR_CH_36_53   ");
      for (i=36; i<54; i++) fscanf(config,"  %i",harray+72*slot+i);
      fscanf(config,"\n"); 

      fscanf(config,"FADC125_THR_CH_54_71   ");
      for (i=54; i<72; i++) fscanf(config,"  %i",harray+72*slot+i);
      fscanf(config,"\n"); 


      fscanf(config,"\n\n"); //2 empty lines


      //fscanf(config,"FADC125_BL_CH_00_17   ");
      fscanf(config,"%s   ",line);
      //printf("BL: %s\n",line);
      if (!strcmp(line,"FADC125_BL_CH_00_17   ")) printf("BL mismatch\n");
      if (!strcmp(line,"FADC125_BL_CH_00_17   ")) break;

      for (i=0; i<18; i++) {
        fscanf(config,"  %f",&fjunk);
        parray[72*slot+i] = (int)fjunk;
      }
      fscanf(config,"\n"); 

      fscanf(config,"FADC125_BL_CH_18_35   ");
      for (i=18; i<36; i++) {
        fscanf(config,"  %f",&fjunk);
        parray[72*slot+i] = (int)fjunk;
      }
      fscanf(config,"\n"); 

      fscanf(config,"FADC125_BL_CH_36_53   ");
      for (i=36; i<54; i++) {
        fscanf(config,"  %f",&fjunk);
        parray[72*slot+i] = (int)fjunk;
      }
      fscanf(config,"\n"); 

      fscanf(config,"FADC125_BL_CH_54_71   ");
      for (i=54; i<72; i++) {
        fscanf(config,"  %f",&fjunk);
        parray[72*slot+i] = (int)fjunk;
      }
      fscanf(config,"\n"); 

      fscanf(config,"\n\n"); //2 empty lines


      //fscanf(config,"FADC125_SIG_CH_00_17   ");
      fscanf(config,"%s   ",line);
      //printf("SIG: %s\n",line);
      if (!strcmp(line,"FADC125_SIG_CH_00_17   ")) printf("SIG mismatch\n");
      if (!strcmp(line,"FADC125_SIG_CH_00_17   ")) break;

      // fscanf(config,"FADC125_SIG_CH_00_17   ");
      for (i=0; i<18; i++) fscanf(config,"  %f",&fjunk);
      fscanf(config,"\n"); 

      fscanf(config,"FADC125_SIG_CH_18_35   ");
      for (i=0; i<18; i++) fscanf(config,"  %f",&fjunk);
      fscanf(config,"\n"); 

      fscanf(config,"FADC125_SIG_CH_36_53   ");
      for (i=0; i<18; i++) fscanf(config,"  %f",&fjunk);
      fscanf(config,"\n"); 

      fscanf(config,"FADC125_SIG_CH_54_71   ");
      for (i=0; i<18; i++) {
         fscanf(config,"  %f",&fjunk);
      }

      fscanf(config,"\n"); //1 empty line


      //fscanf(config,"FADC125_CH_ENB   %*s  %*s  %*s\n");
      fscanf(config,"%s   %*s  %*s  %*s\n",line);
      //printf("ENB: %s\n",line);
      fscanf(config,"\n"); //1 empty line
      //fscanf(config,"FADC125_CH_DIS   %*s  %*s  %*s\n");
      fscanf(config,"%s   %*s  %*s  %*s\n",line);
      //printf("DIS: %s\n",line);
      fscanf(config,"\n"); //1 empty line

      fscanf(config,"FADC125_TH  %i\n",&junk);
      //printf("runconfig TH %i\n",junk);
      fscanf(config,"FADC125_TL  %i\n",&junk);
      //printf("runconfig TL %i\n",junk);

      fscanf(config,"\n"); //1 empty line
    }  // loop over slots
 // end of one roc.

    fclose(config);

    return;

}



void geth(void) {


  Int_t roc25n[19][72] = {0};   //values of straw number, n, for roc25[slot][channel]
  Int_t roc26n[19][72] = {0};
  Int_t roc27n[19][72] = {0};
  Int_t roc28n[19][72] = {0};


  // read list of channels and make index arrays eg roc25n[slot][channel] = n
  readchannellist(roc25n, roc26n, roc27n, roc28n);

  

  Int_t roc25h[19][72] = {0};   //thresholds H
  Int_t roc26h[19][72] = {0};
  Int_t roc27h[19][72] = {0};
  Int_t roc28h[19][72] = {0};

  int thres[3522] = {0}; 

  Int_t *harray,*narray;

  

  for (int iroc=25; iroc<29; iroc++) {

    if (iroc==25) harray = &roc25h[0][0];
    if (iroc==26) harray = &roc26h[0][0];
    if (iroc==27) harray = &roc27h[0][0];
    if (iroc==28) harray = &roc28h[0][0];

    //read in values of H from config file - separate file for each roc
    readconfigfile(iroc,harray,thres);
    
    //printf("\nroc %i\n",iroc);

    if (iroc==25) narray = &roc25n[0][0];
    if (iroc==26) narray = &roc26n[0][0];
    if (iroc==27) narray = &roc27n[0][0];
    if (iroc==28) narray = &roc28n[0][0];

    
    for (int slot=3; slot<18; slot++) {
      
      if (slot==11 || slot==12) continue;

      if (iroc==26 && slot==17) continue;
      if (iroc==27 && slot==17) continue;

      //      printf("slot %i\n",slot);
      
      int startslot = slot*72;  

      for (int i=0; i<72; i++) {

        if (iroc==26 && slot==4 && i>47) continue;

        int n =  *(narray+startslot+i);

        if (n==0) printf("no straw for roc %i slot %i ch %i\n",iroc,slot,i);
        if (n==0) continue;

        thres[n-1] = *(harray+startslot+i);  // H for straw 1 goes into thres[0]

	//        printf("roc %i slot %i ch %i n %i h %i ped %i hit thres %i\n",iroc+24,slot,i,n,*(harray+startslot+i),*(parray+ startslot+i),thres[n]);


      } //chan
   
    } //slot
      
  }  //roc


    
  Char_t filename[500];
  
  sprintf(filename,"cdc_h.txt");
  
  FILE *thr = fopen(filename,"w");
  
  for (int i=0; i<3522; i++) {

    fprintf(thr,"%i\n",thres[i]);
    if (i>9 && i< 3510) continue;
    cout << i+1 << " " << thres[i] << endl;

  }

  fclose(thr);

  printf("Wrote thresholds to %s\n",filename);
   
  //return;
}

