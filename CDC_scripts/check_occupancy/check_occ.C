

void check_occ(const float tolerance=2.0){

  // scan histogram from CDC_online plugin for noisy & quiet straws
  // look for straws with <tolerance> and 1/<tolerance> x the mean number of hits for the ring

  if (tolerance==0) cout << "tolerance has to be non-zero!\n";
  if (tolerance==0) return;


  TDirectory *dir = (TDirectory*)gDirectory->FindObjectAny("CDC");
  if(!dir) return;
	
  dir->cd();

  uint Nstraws[28] = {42, 42, 54, 54, 66, 66, 80, 80, 93, 93, 106, 106, 123, 123, 135, 135, 146, 146, 158, 158, 170, 170, 182, 182, 197, 197, 209, 209};

  //add extra 0 at front to use offset[1] for ring 1
  uint straw_offset[29] = {0,0,42,84,138,192,258,324,404,484,577,670,776,882,1005,1128,1263,1398,1544,1690,1848,2006,2176,2346,2528,2710,2907,3104,3313};

  TH2I *h = (TH2I*)gDirectory->Get("cdc_o");
  if (!h) return;


  // read in list of channel numbers

  FILE *translate = fopen("CDC_straw_numbers_run_3221.txt","r");
  if (!translate) cout << "Could not find file CDC_straw_numbers_run_3221.txt\n";
  if (!translate) return;

  uint ring[3522];
  uint straw[3522];
  uint roc[3522];
  uint slot[3522];
  uint channel[3522];

  char line[200]; 
  
  fscanf(translate,"%*s %*s %s\n",line);  //  # CDC channels
  fscanf(translate,"%*s %*s %*s %*s %*s %*s %s\n",line);  // # N ring straw roc slot channel

  uint n,rg,st,r,s,c;

  while (!feof(translate)) {

    fscanf(translate,"%i %i %i %i %i %i\n",&n,&rg,&st,&r,&s,&c);

    ring[n-1] = rg;
    straw[n-1] = st;
    roc[n-1] = r;
    slot[n-1] = s;
    channel[n-1] = c;

  }

  fclose(translate);


  // scan CDC_online's occupancy histo cdc_o to look for bad straws 
  // cdc_o = new TH2I("cdc_o","CDC occupancy by straw, ring;straw;ring",209,0.5,209.5,28,0.5,28.5);


  float rate[3522] = {0};

  // save straw number and rate ordered by roc slot channel
  uint remapped_n[4][16][72];
  float remapped_rate[4][16][72] = {0};  

  bool foundnoise = 0;

  // Print list of quiet straws on first pass, save rates & id of noisy straws to arrays
  cout << "Quiet straws:\n";

  for (uint iring=1; iring<=28; iring++){

    int total=0;

    for (uint istraw=1; istraw<=Nstraws[iring-1]; istraw++) {
      total += h->GetBinContent(istraw,iring);
    }

    int average = (int)(total/(float)Nstraws[iring-1]);

    for (uint istraw=1; istraw<=Nstraws[iring-1]; istraw++) {

      int n =  istraw + straw_offset[iring];  // straw number

      int nhits = h->GetBinContent(istraw,iring);

      float hitrate = nhits/(float)average;

      sprintf(line," roc %i slot %i channel %i",roc[n-1],slot[n-1],channel[n-1]);

      if (n==709 || n==2384) sprintf(line,"%s (disconnected)",line);
      
      if (nhits > tolerance*average) {
        remapped_n[roc[n-1]-25][slot[n-1]-3][channel[n-1]] = n;
        remapped_rate[roc[n-1]-25][slot[n-1]-3][channel[n-1]] = hitrate;
      }

      if (nhits > tolerance*average) foundnoise=1;
      if (nhits > tolerance*average) rate[n-1] = hitrate; 
      //      if (nhits > tolerance*average) printf("Noisy straw %i ring %i  rate %.1f  %s\n",istraw,iring,hitrate,line);
      if (nhits < average/tolerance) printf("n %4i ring %2i straw %3i  rate %.1f  %s\n",n,iring,istraw,hitrate,line);

    }
    
  }


  cout << "\nNoisy straws:\n";

  if (!foundnoise) cout << "none!\n";
  if (!foundnoise) return;


  for (uint i=1; i<=3522; i++) {
    if (rate[i-1] > 0) printf("n %4i ring %2i straw %3i  rate %.1f  roc %2i slot %2i channel %2i\n",i,ring[i-1],straw[i-1],rate[i-1],roc[i-1],slot[i-1],channel[i-1]);
  }

  cout << "\nNoisy straws, sorted by channel:\n";

  for (uint ir=25; ir<=28; ir++) {
    for (uint is=3; is<20; is++) {
      for (uint ic=0; ic<72; ic++) {

        if (remapped_rate[ir-25][is-3][ic] == 0) continue;

        int n = remapped_n[ir-25][is-3][ic];
        printf("roc %i slot %i channel %i  rate %.1f  n %4i ring %2i straw %2i\n",ir,is,ic, remapped_rate[ir-25][is-3][ic],n, ring[n-1],straw[n-1]);

      }
    }
  } 




}
