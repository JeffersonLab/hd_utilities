#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include "TString.h"
#include "StringUtilities.h"


  // ********************************************************
  // CONVERSIONS FROM STRING TO TSTRING AND BACK
  // ********************************************************

TString
StringUtilities::string2TString(string input){
  TString output("");
  output += input;
  return output;
}

string
StringUtilities::TString2string(TString input){
  string output("");
  for (int i = 0; i < input.Length(); i++){
    output += input[i];
  }
  return output;
}


  // ********************************************************
  // CONVERT STRINGS TO NUMBERS
  // ********************************************************


int
StringUtilities::TString2int(TString input){
  if (input.IsFloat()){
    string sinput = TString2string(input);
    return (int) atof(sinput.c_str());
  }
  TString embeddedNumber("");
  for (int i = 0; i < input.Length(); i++){
    TString digit(input[i]);
    if (digit.IsFloat() || digit == ".")
      embeddedNumber += digit;
  }
  if (!embeddedNumber.IsFloat()) return 0;
  string sinput = TString2string(embeddedNumber);
  return atoi(sinput.c_str());
}

double
StringUtilities::TString2double(TString input){
  if (input.IsFloat()){
    string sinput = TString2string(input);
    return atof(sinput.c_str());
  }
  TString embeddedNumber("");
  for (int i = 0; i < input.Length(); i++){
    TString digit(input[i]);
    if (digit.IsFloat() || digit == ".")
      embeddedNumber += digit;
  }
  if (!embeddedNumber.IsFloat()) return 0.0;
  string sinput = TString2string(embeddedNumber);
  return atof(sinput.c_str());
}  


  // ********************************************************
  // CONVERT FROM INT TO TSTRING
  // ********************************************************

TString 
StringUtilities::int2TString(int number, int ndigits){
  TString snumber("");  snumber += number;
  TString newsnumber("");
  for (int i = snumber.Length(); i < ndigits; i++){ newsnumber += "0"; }
  newsnumber += snumber;
  return newsnumber;
}



  // ********************************************************
  // CONVERT FROM DOUBLE TO TSTRING
  //   if fixdecimal is false:
  //     precision is the number of significant digits
  //   if fixdecimal is true:
  //     the last digit will be in the pow(10,precision) place
  // ********************************************************


TString 
StringUtilities::double2TString(double x, int precision, bool scientific, bool fixdecimal){

  if (fixdecimal) return matchPrecision(x,pow(10.0,precision),1,scientific);

  if (precision == 0){
    if (x >=  5*pow(10.0,getExponent(x,1))) x += 5*pow(10.0,getExponent(x,1));
    if (x <= -5*pow(10.0,getExponent(x,1))) x -= 5*pow(10.0,getExponent(x,1));
    precision = 1;
  }

  stringstream xstream;
  xstream.precision(precision-1);
  xstream.setf(ios::scientific);
  xstream << x;

  TString snumber = StringUtilities::string2TString(xstream.str());

  if (scientific) return snumber;

  int iexponent = getExponent(snumber);

  snumber.Replace(snumber.Index("e"),4,"");

  TString sign("");
  if (snumber.Contains("-")){
    sign = "-";
    snumber.Replace(snumber.Index("-"),1,"");
  }

  while (iexponent > 0){
    if (snumber.Contains(".")){
      int decimal = snumber.Index(".");
      snumber.Replace(decimal,1,"");
      if (decimal < snumber.Length()){
        snumber.Replace(decimal+1,0,".");
      }
      else{
        snumber.Append("0");
      }
    }
    else{
      snumber.Append("0");
    }
    iexponent--;
  }

  while (iexponent < 0){
    if (snumber.Contains(".")){
      snumber.Replace(snumber.Index("."),1,"");
    }
    snumber = ("0."+snumber);
    iexponent++;
  }


  if (snumber.Contains(".") && snumber.Index(".") == snumber.Length()-1)
    snumber.Replace(snumber.Index("."),1,"");

  return (sign+snumber);

}



TString 
StringUtilities::latexMeasurement(double x, double ex1, 
                                  int precision, bool fixdecimal){

  double ex;
  if (!fixdecimal) ex = ex1;
  if (fixdecimal) {ex = pow(10.0,precision); precision = 1;}

  TString sx  = matchPrecision( x,ex,precision,false);
  TString sex = matchPrecision(ex,ex,precision,false);

  return (sx+"\\pm"+sex);

}


TString 
StringUtilities::latexMeasurement(double x, TString sign1, double ex1, 
                                   TString sign2, double ex2, 
                                  int precision, bool fixdecimal){

  double ex;
  if (!fixdecimal) ex = getLeastPrecise(ex1,ex2,precision);
  if (fixdecimal) {ex = pow(10.0,precision); precision = 1;}

  TString sx    = matchPrecision( x, ex,precision,false);
  TString sex1  = matchPrecision(ex1,ex,precision,false);
  TString sex2  = matchPrecision(ex2,ex,precision,false);

  TString a1(""); TString b1("");  if (sign1 == "+-"){ a1 = "\\pm"; b1 = "";}
                                   if (sign1 == "+") { a1 = "^{+";  b1 = "}";}
                                   if (sign1 == "-") { a1 = "_{-";  b1 = "}";}
  TString a2(""); TString b2("");  if (sign2 == "+-"){ a2 = "\\pm"; b2 = "";}
                                   if (sign2 == "+") { a2 = "^{+";  b2 = "}";}
                                   if (sign2 == "-") { a2 = "_{-";  b2 = "}";}

  return (sx+a1+sex1+b1+a2+sex2+b2);

}


TString 
StringUtilities::latexMeasurement(double x, TString sign1, double ex1, 
                                   TString sign2, double ex2,
                                   TString sign3, double ex3, 
                                  int precision, bool fixdecimal){

  double ex;
  if (!fixdecimal) ex = getLeastPrecise(ex1,ex2,ex3,precision);
  if (fixdecimal) {ex = pow(10.0,precision); precision = 1;}

  TString sx    = matchPrecision( x, ex,precision,false);
  TString sex1  = matchPrecision(ex1,ex,precision,false);
  TString sex2  = matchPrecision(ex2,ex,precision,false);
  TString sex3  = matchPrecision(ex3,ex,precision,false);

  TString a1(""); TString b1("");  if (sign1 == "+-"){ a1 = "\\pm"; b1 = "";}
                                   if (sign1 == "+") { a1 = "^{+";  b1 = "}";}
                                   if (sign1 == "-") { a1 = "_{-";  b1 = "}";}
  TString a2(""); TString b2("");  if (sign2 == "+-"){ a2 = "\\pm"; b2 = "";}
                                   if (sign2 == "+") { a2 = "^{+";  b2 = "}";}
                                   if (sign2 == "-") { a2 = "_{-";  b2 = "}";}
  TString a3(""); TString b3("");  if (sign3 == "+-"){ a3 = "\\pm"; b3 = "";}
                                   if (sign3 == "+") { a3 = "^{+";  b3 = "}";}
                                   if (sign3 == "-") { a3 = "_{-";  b3 = "}";}

  return (sx+a1+sex1+b1+a2+sex2+b2+a3+sex3+b3);

}


TString 
StringUtilities::latexMeasurement(double x, TString sign1, double ex1, 
                                   TString sign2, double ex2,
                                   TString sign3, double ex3,
                                   TString sign4, double ex4,
                                  int precision, bool fixdecimal){

  double ex;
  if (!fixdecimal) ex = getLeastPrecise(ex1,ex2,ex3,ex4,precision);
  if (fixdecimal) {ex = pow(10.0,precision); precision = 1;}

  TString sx    = matchPrecision( x, ex,precision,false);
  TString sex1  = matchPrecision(ex1,ex,precision,false);
  TString sex2  = matchPrecision(ex2,ex,precision,false);
  TString sex3  = matchPrecision(ex3,ex,precision,false);
  TString sex4  = matchPrecision(ex4,ex,precision,false);

  TString a1(""); TString b1("");  if (sign1 == "+-"){ a1 = "\\pm"; b1 = "";}
                                   if (sign1 == "+") { a1 = "^{+";  b1 = "}";}
                                   if (sign1 == "-") { a1 = "_{-";  b1 = "}";}
  TString a2(""); TString b2("");  if (sign2 == "+-"){ a2 = "\\pm"; b2 = "";}
                                   if (sign2 == "+") { a2 = "^{+";  b2 = "}";}
                                   if (sign2 == "-") { a2 = "_{-";  b2 = "}";}
  TString a3(""); TString b3("");  if (sign3 == "+-"){ a3 = "\\pm"; b3 = "";}
                                   if (sign3 == "+") { a3 = "^{+";  b3 = "}";}
                                   if (sign3 == "-") { a3 = "_{-";  b3 = "}";}
  TString a4(""); TString b4("");  if (sign4 == "+-"){ a4 = "\\pm"; b4 = "";}
                                   if (sign4 == "+") { a4 = "^{+";  b4 = "}";}
                                   if (sign4 == "-") { a4 = "_{-";  b4 = "}";}

  return (sx+a1+sex1+b1+a2+sex2+b2+a3+sex3+b3+a4+sex4+b4);

}



int 
StringUtilities::getExponent(TString input){

  if (!input.Contains("e") || input.Length() < 4){
    cout << "problem in StringUtilities::getExponent" << endl;
    return 0;
  }

  TString sexponent(""); 
  if (input[input.Length()-3] == '-')
    sexponent += input[input.Length()-3];
  if (input[input.Length()-2] != '0')
    sexponent += input[input.Length()-2];
  sexponent += input[input.Length()-1];

  return StringUtilities::TString2int(sexponent);

}



int 
StringUtilities::getExponent(double x, int precision){

  stringstream xstream;
  xstream.precision(precision-1);
  xstream.setf(ios::scientific);
  xstream << x;

  TString snumber = StringUtilities::string2TString(xstream.str());

  return getExponent(snumber);  

}



TString 
StringUtilities::matchPrecision(double x, double reference, int precision, bool scientific){

  TString sx   = double2TString(x,15,true);
  TString sref = double2TString(reference,precision,true);

  int xprecision = getExponent(sx)-getExponent(sref)+precision;

  if (xprecision >= 0){
    if (getExponent(double2TString(x,xprecision,true)) > getExponent(sx)){
      x = StringUtilities::TString2double(double2TString(x,xprecision,true));
      xprecision++;
    }
  }

  if (xprecision > 0){
    return double2TString(x,xprecision,scientific);
  }

  return double2TString(0.0,1,scientific);

}


double 
StringUtilities::getLeastPrecise(double x1, double x2, int precision){
  double x = x1;
  if (getExponent(double2TString(x, precision,true)) < 
      getExponent(double2TString(x2,precision,true))) x = x2;
  return x;
}

double 
StringUtilities::getLeastPrecise(double x1, double x2, double x3, int precision){
  double x = x1;
  if (getExponent(double2TString(x, precision,true)) < 
      getExponent(double2TString(x2,precision,true))) x = x2;
  if (getExponent(double2TString(x, precision,true)) < 
      getExponent(double2TString(x3,precision,true))) x = x3;
  return x;
}

double 
StringUtilities::getLeastPrecise(double x1, double x2, double x3, double x4, int precision){
  double x = x1;
  if (getExponent(double2TString(x, precision,true)) < 
      getExponent(double2TString(x2,precision,true))) x = x2;
  if (getExponent(double2TString(x, precision,true)) < 
      getExponent(double2TString(x3,precision,true))) x = x3;
  if (getExponent(double2TString(x, precision,true)) < 
      getExponent(double2TString(x4,precision,true))) x = x4;
  return x;
}



  // ********************************************************
  // CONVERT SYMBOLS TO ROOT FORMAT (e.g. "pi+" TO "#pi^{+}")
  // ********************************************************

TString
StringUtilities::rootSymbols(TString input){
  while (input.Contains("\\pm")){
    input.Replace(input.Index("\\pm"),3," #pm ");
  }
  while (input.Contains("omega")){
    input.Replace(input.Index("omega"),5,"TMP");
  }
      while (input.Contains("TMP")){
	input.Replace(input.Index("TMP"),3,"#omega");
      }
  while (input.Contains("phi")){
    input.Replace(input.Index("phi"),3,"TMP");
  }
      while (input.Contains("TMP")){
	input.Replace(input.Index("TMP"),3,"#phi");
      }
  while (input.Contains("etaprime")){
    input.Replace(input.Index("etaprime"),8,"#eta'");
  }
  while (input.Contains("mu+mu-")){
    input.Replace(input.Index("mu+mu-"),6,"#mu^{+}#mu^{-}");
  }
  while (input.Contains("mu+")){
    input.Replace(input.Index("mu+"),3,"#mu^{+}");
  }
  while (input.Contains("mu-")){
    input.Replace(input.Index("mu-"),3,"#mu^{-}");
  }
  while (input.Contains("e+e-")){
    input.Replace(input.Index("e+e-"),4,"e^{+}e^{-}");
  }
  while (input.Contains("e+")){
    input.Replace(input.Index("e+"),2,"e^{+}");
  }
  while (input.Contains("e-")){
    input.Replace(input.Index("e-"),2,"e^{-}");
  }
  while (input.Contains("K+")){
    input.Replace(input.Index("K+"),2,"K^{+}");
  }
  while (input.Contains("K-")){
    input.Replace(input.Index("K-"),2,"K^{-}");
  }
  while (input.Contains("K_S0")){
    input.Replace(input.Index("K_S0"),4,"K_{S}");
  }
  while (input.Contains("Ks")){
    input.Replace(input.Index("Ks"),2,"K_{S}");
  }
  while (input.Contains("pi+")){
    input.Replace(input.Index("pi+"),3,"#pi^{+}");
  }
  while (input.Contains("pi-")){
    input.Replace(input.Index("pi-"),3,"#pi^{-}");
  }
  while (input.Contains("pi0")){
    input.Replace(input.Index("pi0"),3,"#pi^{0}");
  }
  while (input.Contains("eta")){
    input.Replace(input.Index("eta"),3,"TMP");
  }
      while (input.Contains("TMP")){
	input.Replace(input.Index("TMP"),3,"#eta");
      }
  while (input.Contains("#eta+-0")){
    input.Replace(input.Index("#eta+-0"),7,"#eta_{+-0}");
  }
  while (input.Contains("p+")){
    input.Replace(input.Index("p+"),2,"p^{+}");
  }
  while (input.Contains("p-")){
    input.Replace(input.Index("p-"),2,"p^{-}");
  }
  while (input.Contains("gamma")){
    input.Replace(input.Index("gamma"),5,"TMP");
  }
      while (input.Contains("TMP")){
	input.Replace(input.Index("TMP"),3,"#gamma");
      }
  while (input.Contains("ALambda")){
    input.Replace(input.Index("ALambda"),7,"TMP");
  }
      while (input.Contains("TMP")){
	input.Replace(input.Index("TMP"),3,"#Lambda");
      }
  while (input.Contains("Lambda")){
    input.Replace(input.Index("Lambda"),6,"TMP");
  }
      while (input.Contains("TMP")){
	input.Replace(input.Index("TMP"),3,"#Lambda");
      }
  while (input.Contains("##")){
    input.Replace(input.Index("##"),2,"#");
  }
  return input;
}


  // ********************************************************
  // CONVERT SYMBOLS TO LATEX FORMAT (e.g. "pi+" TO "\pi^{+}")
  // ********************************************************

TString
StringUtilities::latexSymbols(TString input){
  while (input.Contains("omega")){
    input.Replace(input.Index("omega"),5,"TMP");
  }
  while (input.Contains("TMP")){
    input.Replace(input.Index("TMP"),3,"\\omega");
  }
  while (input.Contains("phi")){
    input.Replace(input.Index("phi"),3,"TMP");
  }
  while (input.Contains("TMP")){
    input.Replace(input.Index("TMP"),3,"\\phi");
  }
  while (input.Contains("etaprime")){
    input.Replace(input.Index("etaprime"),8,"\\eta^{\\prime}");
  }
  while (input.Contains("mu+mu-")){
    input.Replace(input.Index("mu+mu-"),6,"\\mu^{+}\\mu^{-}");
  }
  while (input.Contains("mu+")){
    input.Replace(input.Index("mu+"),3,"\\mu^{+}");
  }
  while (input.Contains("mu-")){
    input.Replace(input.Index("mu-"),3,"\\mu^{-}");
  }
  while (input.Contains("e+e-")){
    input.Replace(input.Index("e+e-"),4,"e^{+}e^{-}");
  }
  while (input.Contains("e+")){
    input.Replace(input.Index("e+"),2,"e^{+}");
  }
  while (input.Contains("e-")){
    input.Replace(input.Index("e-"),2,"e^{-}");
  }
  while (input.Contains("K+")){
    input.Replace(input.Index("K+"),2,"K^{+}");
  }
  while (input.Contains("K-")){
    input.Replace(input.Index("K-"),2,"K^{-}");
  }
  while (input.Contains("K_S0")){
    input.Replace(input.Index("K_S0"),4,"K_{S}");
  }
  while (input.Contains("Ks")){
    input.Replace(input.Index("Ks"),2,"K_{S}");
  }
  while (input.Contains("pi+")){
    input.Replace(input.Index("pi+"),3,"\\pi^{+}");
  }
  while (input.Contains("pi-")){
    input.Replace(input.Index("pi-"),3,"\\pi^{-}");
  }
  while (input.Contains("pi0")){
    input.Replace(input.Index("pi0"),3,"\\pi^{0}");
  }
  while (input.Contains("eta")){
    input.Replace(input.Index("eta"),3,"TMP");
  }
  while (input.Contains("TMP")){
    input.Replace(input.Index("TMP"),3,"\\eta");
  }
  while (input.Contains("#eta+-0")){
    input.Replace(input.Index("#eta+-0"),7,"\\eta_{+-0}");
  }
  while (input.Contains("p+")){
    input.Replace(input.Index("p+"),2,"p^{+}");
  }
  while (input.Contains("p-")){
    input.Replace(input.Index("p-"),2,"p^{-}");
  }
  while (input.Contains("gamma")){
    input.Replace(input.Index("gamma"),5,"TMP");
  }
  while (input.Contains("TMP")){
    input.Replace(input.Index("TMP"),3,"\\gamma");
  }
  while (input.Contains("\\\\")){
    input.Replace(input.Index("\\\\"),2,"\\");
  }
  return input;
}



  // ********************************************************
  // PARSE STRINGS INTO WORDS
  // ********************************************************

vector<TString>
StringUtilities::parseString(string input, string spacer){
  return parseTString(string2TString(input),string2TString(spacer));
}

vector<TString>
StringUtilities::parseTString(TString input, TString spacer){
  if (spacer == " ") input = StringUtilities::removeTabs(input);
  vector<TString> words;
  TString word("");
  for (int i = 0; i < input.Length(); i++){
    TString digit(input[i]);
    if ((digit == spacer) || (spacer == " " && digit == "\t")){
      if (word != ""){
        words.push_back(word);
        word = "";
      }
    }
    else{
      word += digit;
      if (i == input.Length()-1){
        words.push_back(word);
      }
    }
  }
  return words;
}


int
StringUtilities::parseTStringSize(TString input, TString spacer){
  return parseTString(input,spacer).size();
}

TString
StringUtilities::parseTStringElement(TString input, int element, TString spacer){
  return parseTString(input,spacer)[element];
}

void
StringUtilities::parseStringTest(string input, string spacer){
  parseTStringTest(string2TString(input), string2TString(spacer));
}

void
StringUtilities::parseTStringTest(TString input, TString spacer){
  cout << "parseTStringTest:" << endl;
  vector<TString> words = parseTString(input,spacer);
  for (unsigned int i = 0; i < words.size(); i++){
    cout << words[i] << endl;
  }
}



  // ********************************************************
  // PARSE SIMPLE LOGIC 
  //    (using "," for OR, "&" for AND, "!" for NOT)
  //    outer vector contains "OR"s
  //    inner vector contains "AND"s
  //    pair.first has int of 1 for NOT
  //    pair.second is a statemet
  // ********************************************************


vector< vector< pair<int,TString> > >
StringUtilities::parseLogicalTString(TString input){
  vector< pair<int,TString> >            andstrings;
  vector< vector< pair<int,TString> > >  orstrings;
  TString cc("");
  int inot = 0;
  for (int ic = 0; ic < input.Length(); ic++){
    TString whitecheck(input[ic]);
    //if (!whitecheck.IsWhitespace()){
    if (whitecheck != ' '){
      if (input[ic] != '&' && input[ic] != ',' && (input[ic] != '!')) { 
	cc += input[ic]; 
      }
      else if (input[ic] == '!') {
	inot = 1;
      } 
      else if (input[ic] == '&') { 
	andstrings.push_back(pair<int,TString>(inot,cc));  
	cc = ""; 
	inot = 0;
      }
      else if (input[ic] == ',') { 
	andstrings.push_back(pair<int,TString>(inot,cc));  
	orstrings.push_back(andstrings);  
	andstrings.clear(); 
	cc = ""; 
	inot = 0;
      }
    }
  }
  andstrings.push_back(pair<int,TString>(inot,cc));
  orstrings.push_back(andstrings);
  return orstrings;
}


void
StringUtilities::parseLogicalTStringTest(TString input){
  cout << "parseLogicalTStringTest:" << endl;
  vector< vector< pair<int,TString> > > orlogic = parseLogicalTString(input);
  for (unsigned int ior = 0; ior < orlogic.size(); ior++){
    vector< pair<int,TString> > andlogic = orlogic[ior];
    for (unsigned int iand = 0; iand < andlogic.size(); iand++){
      TString cnot("");
      if (andlogic[iand].first == 1) cnot += " NOT ";
      cout << cnot << andlogic[iand].second;
      if (iand != andlogic.size()-1) cout << " AND ";
      else cout << endl;
    }
    if (ior != orlogic.size()-1) cout << "OR" << endl;
  }
}


  // ********************************************************
  // LATEX UTILITIES
  // ********************************************************

void
StringUtilities::latexHeader(TString filename, bool append){
  std::_Ios_Openmode mode = ios::out;
  if (append) mode = ios::app;
  ofstream outfile(TString2string(filename).c_str(),mode);
  outfile << "\\documentclass[11pt]{article}" << endl;
  outfile << "\\usepackage{graphicx}" << endl;
  outfile << "\\usepackage{amssymb}" << endl;
  outfile << "\\usepackage[]{epsfig}" << endl;
  outfile << endl;
  outfile << "\\textheight 8.0in" << endl;
  outfile << "\\topmargin 0.0in" << endl;
  outfile << "\\textwidth 6.0in" << endl;
  outfile << "\\oddsidemargin 0.25in" << endl;
  outfile << "\\evensidemargin 0.25in" << endl;
  outfile << endl;
  outfile << "\\newcommand{\\gevc}{\\mathrm{GeV/c}}" << endl;
  outfile << endl;
  outfile << "\\setlength{\\parindent}{0pt}" << endl;
  outfile << "\\setlength{\\parskip}{11pt}" << endl;
  outfile << endl;
  outfile << "\\begin{document}" << endl;
  outfile << endl;
  outfile << "\%\\title{}" << endl;
  outfile << "\%\\author{}" << endl;
  outfile << "\%\\maketitle" << endl;
  outfile << endl;
  outfile << "\%\\abstract{}" << endl;
  outfile.close();
}


void
StringUtilities::latexFigure(TString filename, TString figurename, 
                             TString width, TString caption, bool append){
  std::_Ios_Openmode mode = ios::out;
  if (append) mode = ios::app;
  ofstream outfile(TString2string(filename).c_str(),mode);
  outfile << endl;
  //outfile << "\\newpage" << endl;
  outfile << "\\begin{figure}[htb]" << endl;
  outfile << "\\includegraphics*[width= " << width <<"\\columnwidth]{"
                                          << figurename << "}" << endl;
  outfile << "\\caption{" << caption << "}" << endl;
  outfile << "\\end{figure}" << endl;
  outfile << endl;
  outfile.close();
}

void
StringUtilities::latexLine(TString filename, TString text, bool append){
  std::_Ios_Openmode mode = ios::out;
  if (append) mode = ios::app;
  ofstream outfile(TString2string(filename).c_str(),mode);
  //outfile << endl;
  outfile << text << endl;
  //outfile << endl;
  outfile.close();
}

void
StringUtilities::latexCloser(TString filename, bool append){
  std::_Ios_Openmode mode = ios::out;
  if (append) mode = ios::app;
  ofstream outfile(TString2string(filename).c_str(),mode);
  outfile << endl;
  outfile << "\\end{document}" << endl;
  outfile << endl;
  outfile.close();
}


  // ********************************************************
  // MAKE A LATEX TABLE
  //   pass in a 2d array with tableContents[nrows][ncols]
  //    or pass in a 1d array with tableContents[ncols*nrows]
  //      that fills left to right, top to bottom
  // ********************************************************

void
StringUtilities::latexTable(int nrows, int ncols, TString* tableContents, 
                            TString filename, bool append){
  std::_Ios_Openmode mode = ios::out;
  if (append) mode = ios::app;
  ofstream outfile(TString2string(filename).c_str(),mode);
  outfile << "\\begin{tabular}{|"; for (int k = 0; k < ncols; k++){ outfile << "c|"; } outfile << "}" << endl;
  outfile << "\\hline" << endl;
  int i = 0;
  for (int j = 0; j < nrows; j++){
    for (int k = 0; k < ncols; k++){
      outfile << tableContents[i++] << "  ";
      if (k != ncols - 1) outfile << "&  ";
    }
    outfile << "\\\\" << endl;
    outfile << "\\hline" << endl;
  }
  outfile << "\\end{tabular}" << endl;
  outfile << endl;
  outfile.close();
}  


  // ********************************************************
  // READ A TSTRING FROM A FILE
  // ********************************************************

TString
StringUtilities::readTStringFromFile(TString filename, int line, int word){
  ifstream infile(filename.Data());
  if (!infile) cout << "can't find " << filename << endl;
  string instring;
  for (int i = 0; i < line; i++){ getline(infile,instring); }
  infile.close();
  vector<TString> words = parseString(instring);
  if (word >= (int)words.size()) return TString("");
  if (word < -1*(int)words.size()) return TString("");
  if (word < 0) word += (int)words.size();
  return words[(unsigned int)word];
}


  // *************************************************************
  // MAKE ALL PERMUTATIONS OF INPUTSTRING USING DEFINITIONS
  //   definitions has the format:
  //     NAME1 DEFN11 DEFN12 DEFN13
  //     NAME2 DEFN21 DEFN22 DEFN23 DEFN24
  //     NAME3 DEFN31 DEFN32
  // *************************************************************

vector<TString> 
StringUtilities::expandDefinitions(TString inputString, 
                                   map< TString, vector<TString> > definitions,
                                   TString spacer, bool synchronizeDefinitions){

  vector<TString> expanded;
  expanded.push_back(inputString);


    // first order the definitions map from most to least fundamental
    //  (only makes a difference for synchronizeDefinitions = true)
    //  (use a vector to control the order)

  vector< pair<TString, vector<TString> > > defVector;
  while (!definitions.empty()){
    bool independent = true;
    for (map< TString, vector<TString> >::iterator mapItr = definitions.begin();
         mapItr != definitions.end(); mapItr++){
      independent = true;
      vector<TString> vMeanings = mapItr->second;
      for (unsigned int i = 0; i < vMeanings.size(); i++){
        if (definitions.find(vMeanings[i]) != definitions.end()){
          independent = false;
          break;
        }
      }
      if (independent){
        defVector.push_back(pair<TString,vector<TString> >(mapItr->first,mapItr->second));
        definitions.erase(mapItr);
        break;
      }
    }
    if (!independent){
      cout << "StringUtilities::expandDefinitions...";
      cout << "check for circular definitions...  " << endl;
      return vector<TString>();
    }
  }


    // if using whitespace as the spacer, remove all tabs

  if (spacer == " "){
    for (unsigned int i = 0; i < defVector.size(); i++){
      defVector[i].first = StringUtilities::removeTabs(defVector[i].first);
      for (unsigned int j = 0; j < defVector[i].second.size(); j++){
        defVector[i].second[j] = StringUtilities::removeTabs(defVector[i].second[j]);
      }
    }
  }


    // expand definitions

  bool done = false;
  int iteration = 0;
  while (!done){
    done = true;

      // look for an element of "expanded" that contains a definition

    vector<TString>::iterator vIter;
    vector< pair< TString, vector<TString> > >::reverse_iterator mIter;
    for (vIter = expanded.begin(); 
          vIter != expanded.end(); vIter++){
      for (mIter = defVector.rbegin(); mIter != defVector.rend(); mIter++){
        TString element(spacer+*vIter+spacer);
        if (element.Contains(spacer+(*mIter).first+spacer)) break;
      }
      if (mIter != defVector.rend()) break;
    }

      // if one is found, remove it and add new elements with 
      //   expanded definitions, then repeat

    if (vIter != expanded.end()){
      done = false;
      TString element = *vIter;
      expanded.erase(vIter);
      for (unsigned int i = 0; i < (*mIter).second.size(); i++){
        TString newElement(element);
        if (!synchronizeDefinitions){
          TString newElementCopy(spacer+newElement+spacer);
          int index = newElementCopy.Index(spacer+(*mIter).first+spacer);
          newElement.Replace(index,(*mIter).first.Length(), (*mIter).second[i]);
          expanded.push_back(newElement);
        }
        if (synchronizeDefinitions){
          TString newElementCopy(spacer+newElement+spacer);
          while (newElementCopy.Contains(spacer+(*mIter).first+spacer)){
            int index = newElementCopy.Index(spacer+(*mIter).first+spacer);
            newElement.Replace(index,(*mIter).first.Length(), (*mIter).second[i]);
            newElementCopy = spacer+newElement+spacer;
          }
          expanded.push_back(newElement);
        }
      }
    }

      // infinite loop check

    if (++iteration > 10000){
      cout << "StringUtilities::expandDefinitions...  ";
      cout << "quitting after 10000 iterations...  ";
      cout << "check for an infinite loop... " << endl;
      return vector<TString>();
    }

  }

  return expanded;

}


  // ********************************************************
  // PARSE HISTOGRAM BOUNDS
  // ********************************************************

int    
StringUtilities::parseBoundsNBinsX(TString bounds){
  vector<TString> boundVector = parseBounds(bounds);
  if (boundVector.size() < 3) return 0;
  return TString2int(boundVector[0]);
}

double
StringUtilities::parseBoundsLowerX(TString bounds){
  vector<TString> boundVector = parseBounds(bounds);
  if (boundVector.size() < 3) return 0.0;
  return TString2double(boundVector[1]);
}

double
StringUtilities::parseBoundsUpperX(TString bounds){
  vector<TString> boundVector = parseBounds(bounds);
  if (boundVector.size() < 3) return 0.0;
  return TString2double(boundVector[2]);
}

int    
StringUtilities::parseBoundsNBinsY(TString bounds){
  vector<TString> boundVector = parseBounds(bounds);
  if (boundVector.size() != 6) return 0;
  return TString2int(boundVector[3]);
}

double
StringUtilities::parseBoundsLowerY(TString bounds){
  vector<TString> boundVector = parseBounds(bounds);
  if (boundVector.size() != 6) return 0.0;
  return TString2double(boundVector[4]);
}

double
StringUtilities::parseBoundsUpperY(TString bounds){
  vector<TString> boundVector = parseBounds(bounds);
  if (boundVector.size() != 6) return 0.0;
  return TString2double(boundVector[5]);
}

vector<TString>
StringUtilities::parseBounds(TString bounds){
  vector<TString> boundVector = parseTString(bounds,",");
  vector<TString> emptyVector;
  if ((boundVector.size() != 3 && boundVector.size() != 6) ||
      (boundVector[0].Index("(") != 0) ||
      (boundVector[boundVector.size()-1].Index(")") !=
       boundVector[boundVector.size()-1].Length()-1)){
    cout << "StringUtilities: wrong histogram bounds format " << bounds << endl;
    return emptyVector;
  }
  boundVector[0].Replace(boundVector[0].Index("("),1,"");
  boundVector[boundVector.size()-1].Replace(boundVector[boundVector.size()-1].Index(")"),1,"");
  return boundVector;
}
 


  // ********************************************************
  // CHECK OPENING AND CLOSING PARENTHESES
  // ********************************************************

bool
StringUtilities::checkParentheses(TString input){
  int pcount = 0;
  for (int i = 0; i < input.Length(); i++){
    if ((TString)input[i] == "(") pcount++;
    if ((TString)input[i] == ")") pcount--;
  }
  if (pcount == 0) return true;
  cout << "StringUtilities:  OPENING AND CLOSING PARENTHESES ";
  cout << "DO NOT MATCH IN" << endl;
  cout << "\t" << input << endl;
  return false;
}


  // ********************************************************
  // STRIP EXTRA WHITESPACE AROUND STRINGS AND REMOVE TABS
  // ********************************************************

TString
StringUtilities::stripWhiteSpace(TString input){
  int istart = -1;
  int iend = -1;
  for (int j = 0; j < input.Length(); j++){
    TString digit(input[j]);
    if (!digit.IsWhitespace() && istart == -1) istart = j;
  }
  for (int j = input.Length(); j > 0; j--){
    TString digit(input[j-1]);
    if (!digit.IsWhitespace() && iend == -1) iend = j;
  }
  TString output("");
  for (int j = istart; j < iend; j++){
    output += input[j];
  }
  return output;
}

TString
StringUtilities::removeTabs(TString input){
  TString output("");
  for (int i = 0; i < input.Length(); i++){
    TString digit(input[i]);
    if (digit.IsWhitespace() || digit == "\t"){ output += " ";}
    else{ output += digit; }
  }
  return output;
}

