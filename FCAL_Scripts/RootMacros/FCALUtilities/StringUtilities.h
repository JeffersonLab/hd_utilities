#if !defined(STRINGUTILITIES_H)
#define STRINGUTILITIES_H

#include <cmath>
#include <cstdlib>
#include <vector>
#include <utility>
#include <string>
#include <map>
#include "TString.h"

using namespace std;


class StringUtilities{

  public:

      // ********************************************************
      // CONVERSIONS FROM STRING TO TSTRING AND BACK
      // ********************************************************

    static TString string2TString(string input);
    static string TString2string(TString input);


      // ********************************************************
      // CONVERT FROM TSTRING TO INT
      // ********************************************************

    static int TString2int(TString input);


      // ********************************************************
      // CONVERT FROM TSTRING TO DOUBLE
      // ********************************************************

    static double TString2double(TString input);


      // ********************************************************
      // CONVERT FROM INT TO TSTRING
      // ********************************************************

    static TString int2TString(int number, int ndigits = 0);


      // ********************************************************
      // CONVERT FROM DOUBLE TO TSTRING
      //   if fixdecimal is false:
      //     precision is the number of significant digits
      //   if fixdecimal is true:
      //     the last digit will be in the pow(10,precision) place
      // ********************************************************

    static TString double2TString(double x, int precision = 3, bool scientific = false, 
                                                               bool fixdecimal = false);

    static TString latexMeasurement(double x, double ex1, 
                                    int precision = 2, bool fixdecimal = false);

    static TString latexMeasurement(double x, TString sign1, double ex1, 
                                              TString sign2, double ex2, 
                                    int precision = 2, bool fixdecimal = false);

    static TString latexMeasurement(double x, TString sign1, double ex1, 
                                	      TString sign2, double ex2,
                                	      TString sign3, double ex3,
                                    int precision = 2, bool fixdecimal = false);

    static TString latexMeasurement(double x, TString sign1, double ex1, 
                                	      TString sign2, double ex2,
                                	      TString sign3, double ex3,
                                	      TString sign4, double ex4,
                                    int precision = 2, bool fixdecimal = false);


      // ********************************************************
      // CONVERT SYMBOLS TO ROOT FORMAT (e.g. "pi+" TO "#pi^{+}")
      // ********************************************************

    static TString rootSymbols(TString input);


      // ********************************************************
      // CONVERT SYMBOLS TO LATEX FORMAT (e.g. "pi+" TO "\pi^{+}")
      // ********************************************************

    static TString latexSymbols(TString input);


      // ********************************************************
      // PARSE STRINGS INTO WORDS
      // ********************************************************

    static vector<TString> parseString(string input, string spacer = " ");
    static vector<TString> parseTString(TString input, TString spacer = " ");
    static int parseTStringSize(TString input, TString spacer = " ");
    static TString parseTStringElement(TString input, int element, TString spacer = " ");

    static void parseStringTest(string input, string spacer = " ");
    static void parseTStringTest(TString input, TString spacer = " ");


      // ********************************************************
      // PARSE SIMPLE LOGIC 
      //    (using "," for OR, "&" for AND, "!" for NOT)
      //    outer vector contains "OR"s
      //    inner vector contains "AND"s
      //    pair.first has int of 1 for NOT
      //    pair.second is a statemet
      // ********************************************************

    static vector< vector< pair<int,TString> > >  parseLogicalTString(TString input);

    static void parseLogicalTStringTest(TString input);


      // ********************************************************
      // LATEX UTILITIES
      // ********************************************************

    static void latexHeader(TString filename, bool append = false);
    static void latexFigure(TString filename, TString figurename, 
                            TString width = "1.0", TString caption = "", 
                            bool append = true);
    static void latexLine(TString filename, TString text, bool append = true);
    static void latexCloser(TString filename, bool append = true);


      // ********************************************************
      // MAKE A LATEX TABLE
      //   pass in a 2d array with tableContents[nrows][ncols]
      //    or pass in a 1d array with tableContents[ncols*nrows]
      //      that fills left to right, top to bottom
      // ********************************************************

    static void latexTable(int nrows, int ncols, TString* tableContents, 
                           TString filename, bool append = false);


      // ********************************************************
      // READ A TSTRING FROM A FILE
      // ********************************************************

    static TString readTStringFromFile(TString filename, int line, int word);


      // ********************************************************
      // MAKE ALL PERMUTATIONS OF INPUTSTRING USING DEFINITIONS
      //   definitions has the format:
      //     NAME1 DEFN11 DEFN12 DEFN13
      //     NAME2 DEFN21 DEFN22 DEFN23 DEFN24
      //     NAME3 DEFN31 DEFN32
      // ********************************************************

    static vector<TString> expandDefinitions(TString inputString, 
                                             map<TString, vector<TString> > definitions,
                                             TString spacer = " ",
                                             bool synchronizeDefinitions = false);


      // ********************************************************
      // PARSE HISTOGRAM BOUNDS
      // ********************************************************

    static int    parseBoundsNBinsX(TString bounds);
    static double parseBoundsLowerX(TString bounds);
    static double parseBoundsUpperX(TString bounds);
    static int    parseBoundsNBinsY(TString bounds);
    static double parseBoundsLowerY(TString bounds);
    static double parseBoundsUpperY(TString bounds);


      // ********************************************************
      // CHECK OPENING AND CLOSING PARENTHESES
      // ********************************************************

    static bool checkParentheses(TString input);


      // ********************************************************
      // STRIP EXTRA WHITESPACE AROUND STRINGS AND REMOVE TABS
      // ********************************************************

    static TString stripWhiteSpace(TString input);
    static TString removeTabs(TString input);


  private:

    static vector<TString> parseBounds(TString bounds);


      // helper functions for double2TString

    static int getExponent(TString input);

    static int getExponent(double x, int precision);

    static TString matchPrecision(double x, double reference, int precision, bool scientific);

    static double getLeastPrecise(double x1, double x2, int precision);

    static double getLeastPrecise(double x1, double x2, double x3, int precision);

    static double getLeastPrecise(double x1, double x2, double x3, double x4, int precision);


};



#endif

