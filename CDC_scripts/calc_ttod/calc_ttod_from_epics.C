// Estimate CDC time to distance parameters for CCDB /CDC/drift_parameters for 2125V runs using functions from GlueX-doc-5394
// Arguments are run number (for output filename), uncalibrated pressure and downstream thermocouple temps excluding D2
// NSJ 14 June 2022

void calc_ttod_from_epics(int run, float p, float t1, float t3, float t4, float t5) {

  float calp = 1.064*p - 5.098;    // convert Hall D gas panel reading into kPa following https://logbooks.jlab.org/entry/3810976
  float t = 273.15 + 0.25*(t1 + t3 + t4 + t5);    // convert from C to K 

  
  float d = calp/t;   // kPa/K
  printf("density %.3f\n",d);

  
  /*   // fitted vs uncalibrated pressure from EPICS

  float a1= 2.1053 + -0.0109 *p;
  float b1 = -1.3129 + 0.0120 *p;
  float c1 = 0.6935 + -0.0068 *p;
  float a2 = -2.6136 + 0.0255 *p;
  float b2 = 3.4686 + -0.0394 *p;
  float c2 = -2.7263 + 0.0297 *p;
  */

  
  // fitted vs calibrated pressure/temperature
  
  float a1 = 2.0157 + -2.9468 *d;
  float b1 = -1.1960 + 3.1979 *d;
  float c1 = 0.6225 + -1.8051 *d;
  float a2 = -2.3818 + 6.8484 *d;
  float b2 = 3.1014 + -10.5439 *d;
  float c2 = -2.4503 + 7.9724 *d;

 
  FILE *pfile = fopen(Form("ttod_d%i.txt",run),"w");
  fprintf(pfile,"%.6f %.6f 0 %.6f %.6f 0 %.6f %.6f 0 1.1 -0.08\n",a1,a2,b1,b2,c1,c2);
  fprintf(pfile,"%.6f %.6f 0 %.6f %.6f 0 %.6f %.6f 0 1.1 -0.08\n",a1,-1*a2,b1,-1*b2,c1,-1*c2);
  fclose(pfile);

}
