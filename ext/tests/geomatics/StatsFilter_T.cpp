//==============================================================================
//
//  This file is part of GPSTk, the GPS Toolkit.
//
//  The GPSTk is free software; you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published
//  by the Free Software Foundation; either version 3.0 of the License, or
//  any later version.
//
//  The GPSTk is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with GPSTk; if not, write to the Free Software Foundation,
//  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
//  
//  Copyright 2004-2019, The University of Texas at Austin
//
//==============================================================================

//==============================================================================
//
//  This software developed by Applied Research Laboratories at the University of
//  Texas at Austin, under contract to an agency or agencies within the U.S. 
//  Department of Defense. The U.S. Government retains all rights to use,
//  duplicate, distribute, disclose, or release this software. 
//
//  Pursuant to DoD Directive 523024 
//
//  DISTRIBUTION STATEMENT A: This software has been approved for public 
//                            release, distribution is unlimited.
//
//==============================================================================

/// @file StatsFilter_T.cpp Test classes in StatsFilter.hpp

#include <vector>
#include "FirstDiffFilter.hpp"
#include "FDiffFilter.hpp"
#include "WindowFilter.hpp"
#include "logstream.hpp"

//------------------------------------------------------------------------------------
using namespace std;
using namespace gpstk;

//------------------------------------------------------------------------------------
int testFirstDiff(const vector<double>& xdata,
                  const vector<double>& data,
                  const double& ratlimit,
                  const string& label,
                  const bool& verbose,
                  vector< FilterHit<double> >& hit)
{
   int iret;
   unsigned int i,j,k;

   // fill flags
   vector<int> flags;
   flags = vector<int>(data.size(),0);

   // xdata and flags must exist but may be empty
   FirstDiffFilter<double> fdf(xdata, data, flags);
   fdf.setw(7);
   fdf.setprecision(4);
   fdf.setLimit(ratlimit);
   iret = fdf.filter();
   if(verbose) cout << "# FD Filter returns " << iret << endl;
   if(iret < 0)
      cout << "# FD Filter failed (" << iret << ")" << endl;
   else {
      iret = fdf.analyze();
      if(iret < 0) cout << "# FD Filter analysis failed (" << iret << ")" << endl;
      else iret=0;

      for(i=0; i<fdf.results.size(); i++)
         fdf.getStats(fdf.results[i]);
      fdf.setDumpNoAnal(false);
      if(verbose) fdf.dump(cout, label);

      hit = fdf.getResults();

      // clean the data based on results of filter
      //vector< FilterHit<double> > hit=fdf.getResults();
      for(j=0; j<hit.size(); j++) {
         cout << label << " " << hit[j].asString() << endl;

         //if(hit[j].type == FilterHit<double>::BOD) continue;
         //else if(hit[j].type == FilterHit<double>::outlier) {
         //   for(k=0; k<hit[j].npts; k++)
         //      flags[hit[j].index+k] = -1;      // flag for outlier
         //}
         //else if(hit[j].type == FilterHit<double>::slip) {
         //   for(k=hit[j].index; k<data.size(); k++)
         //      data[k] -= hit[j].step;
         //}
      }

      //// write cleaned data to rstats.out
      //ostream *pout = new ofstream("rstats.out");
      //if(pout->fail()) {
      //   cout << "Unable to open file rstats.out - output to screen\n";
      //   pout = &cout;
      //}

      //for(i=0; i<data.size(); i++)
      //   *pout << fixed << setprecision(prec) << i
      //         << " " << (xdata.size() ? xdata[i] : (double)(i))
      //         << " " << data[i] << " " << flags[i] << endl;
      //if(pout != &cout) ((ofstream *)pout)->close();
   }

   return iret;
}

//------------------------------------------------------------------------------------
int testWindow(const vector<double>& xdata,
               const vector<double>& data,
               const bool& useTSS,
               const double& window,
               const double& steplimit,
               const double& ratlimit,
               const string& label,
               const bool& verbose,
               vector< FilterHit<double> >& hit)
{
   int iret;
   unsigned int i,j,k;

   // fill flags
   vector<int> flags;
   flags = vector<int>(data.size(),0);

   // one-sample stats
   WindowFilter<double> wf(xdata, data, flags);
   wf.setTwoSample(useTSS);
   wf.setWidth(window);
   if(ratlimit > 0.0) wf.setMinRatio(ratlimit);
   if(steplimit > 0.0) wf.setMinStep(steplimit);
   wf.setw(7);
   wf.setprecision(4);
   iret = wf.filter();
   if(iret < 0) cout << "# window filter failed (" << iret << ")" << endl;
   else {
      if(verbose) wf.setDebug(true);
      wf.analyze();           // ignore return values
      if(verbose) wf.setDumpAnalMsg(true);
      if(verbose) wf.dump(cout, label);

      hit = wf.getResults();
      for(j=0; j<hit.size(); j++)
         cout << label << " " << hit[j].asString() << endl;
   }

   return iret;
}

//------------------------------------------------------------------------------------
int testFDiff(const vector<double>& xdata,
              const vector<double>& data,
              const double& ratlimit,
              const string& label,
              const bool& verbose,
              vector< FilterHit<double> >& hit)
{
   int iret;
   unsigned int i,j,k;

   // fill flags
   vector<int> flags;
   flags = vector<int>(data.size(),0);

   // xdata and flags must exist but may be empty
   IterativeFDiffFilter<double> fdf(xdata, data, flags);
   fdf.setw(7);
   fdf.setprecision(4);

   fdf.setWidth(4);
   fdf.setLimit(0.8);
   fdf.setSigma(ratlimit);

   fdf.doVerbose(verbose);
   fdf.doResetSigma(true);
   fdf.doSmallSlips(false);

   iret = fdf.analysis();
   if(iret < 0) cout << "# FDiffFilter analysis failed (" << iret << ")" << endl;
   else iret=0;

   hit = fdf.getResults();

   // clean the data based on results of filter
   //vector< FilterHit<double> > hit=fdf.getResults();
   for(j=0; j<hit.size(); j++) {
      cout << label << " " << hit[j].asString() << endl;

      //if(hit[j].type == FilterHit<double>::BOD) continue;
      //else if(hit[j].type == FilterHit<double>::outlier) {
      //   for(k=0; k<hit[j].npts; k++)
      //      flags[hit[j].index+k] = -1;      // flag for outlier
      //}
      //else if(hit[j].type == FilterHit<double>::slip) {
      //   for(k=hit[j].index; k<data.size(); k++)
      //      data[k] -= hit[j].step;
      //}
   }

   //// write cleaned data to rstats.out
   //ostream *pout = new ofstream("rstats.out");
   //if(pout->fail()) {
   //   cout << "Unable to open file rstats.out - output to screen\n";
   //   pout = &cout;
   //}

   //for(i=0; i<data.size(); i++)
   //   *pout << fixed << setprecision(prec) << i
   //         << " " << (xdata.size() ? xdata[i] : (double)(i))
   //         << " " << data[i] << " " << flags[i] << endl;
   //if(pout != &cout) ((ofstream *)pout)->close();

   return iret;
}

//------------------------------------------------------------------------------------
int main(int argc, char **argv)
{
try {
   // datasets
   //# Satellites G12
   //# Linear combinations WLC:12 GF:P:12
   //# RinDump output for file job4131.obs
   //# Header ObsIDs GPS (4): C1C L1C C2W L2X
   //# wk secs-of-wk sat     WLC:12       GF:P:12   
   const unsigned int M1=243;
   // sow WLC GF
   double data1[3*M1] = {
    485370.000, -0.642, 0.050, 485400.000, -0.586, 0.085, 485430.000, -0.415, 0.127,
    485460.000, -0.374, 0.163, 485490.000, -0.426, 0.191, 485520.000, -0.567, 0.223,
    485550.000, -0.855, 0.244, 485580.000, -1.151, 0.265, 485610.000, -1.329, 0.288,
    485640.000, -1.369, 0.312, 485670.000, -1.270, 0.328, 485700.000, -0.956, 0.348,
    485730.000, -0.585, 0.369, 485760.000, -0.368, 0.386, 485790.000, -0.324, 0.408,
    485820.000, -0.374, 0.428, 485850.000, -0.487, 0.441, 485880.000, -0.631, 0.451,
    485910.000, -0.754, 0.469, 485940.000, -0.884, 0.475, 485970.000, -0.974, 0.491,
    486000.000, -1.103, 0.499, 486030.000, -1.142, 0.504, 486060.000, -0.872, 0.505,
    486090.000, -0.414, 0.495, 486120.000, -0.006, 0.495, 486150.000,  0.300, 0.516,
    486180.000,  0.335, 0.522, 486210.000,  0.156, 0.520, 486240.000, -0.106, 0.505,
    486270.000, -0.313, 0.502, 486300.000, -0.651, 0.494, 486330.000, -1.062, 0.489,
    486360.000, -1.381, 0.490, 486390.000, -1.552, 0.486, 486420.000, -1.427, 0.481,
    486450.000, -0.992, 0.478, 486480.000, -0.536, 0.468, 486510.000, -0.211, 0.468,
    486540.000, -0.212, 0.467, 486570.000, -0.436, 0.467, 486600.000, -0.803, 0.466,
    486630.000, -1.141, 0.478, 486660.000, -1.488, 0.481, 486690.000, -1.708, 0.478,
    486720.000, -1.685, 0.459, 486750.000, -1.529, 0.454, 486780.000, -1.521, 0.455,
    486810.000, -1.540, 0.473, 486840.000, -1.509, 0.490, 486870.000, -1.422, 0.506,
    486900.000, -1.459, 0.497, 487020.000, -1.860,-0.438, 487050.000, -2.144,-0.422,
    487080.000, -1.880,-0.400, 487110.000, -1.442,-0.399, 487140.000, -1.077,-0.425,
    487170.000, -0.669,-0.406, 487200.000, -0.653,-0.402, 487230.000, -0.626,-0.385,
    487260.000, -0.523,-0.379, 487290.000, -0.456,-0.387, 487320.000, -0.555,-0.379,
    487350.000, -0.821,-0.371, 487380.000, -1.135,-0.361, 487410.000, -1.237,-0.366,
    487440.000, -1.019,-0.372, 487470.000, -0.807,-0.370, 487500.000, -0.879,-0.369,
    487530.000, -1.109,-0.356, 487560.000, -1.390,-0.354, 487590.000, -1.524,-0.351,
    487620.000, -1.403,-0.363, 487650.000, -1.240,-0.361, 487680.000, -1.309,-0.365,
    487710.000, -1.449,-0.357, 487740.000, -1.613,-0.358, 487770.000, -1.677,-0.355,
    487800.000, -1.598,-0.358, 487830.000, -1.395,-0.364, 487860.000, -1.180,-0.365,
    487890.000, -1.048,-0.361, 487920.000, -1.040,-0.357, 487950.000, -1.101,-0.361,
    487980.000, -1.105,-0.369, 488010.000, -0.993,-0.371, 488040.000, -0.747,-0.362,
    488070.000, -0.558,-0.370, 488100.000, -0.392,-0.375, 488130.000, -0.321,-0.376,
    488160.000, -0.363,-0.375, 488190.000, -0.399,-0.370, 488220.000, -0.406,-0.387,
    488250.000, -0.277,-0.389, 488280.000, -0.324,-0.390, 488310.000, -0.473,-0.389,
    488340.000, -0.613,-0.395, 488370.000, -0.660,-0.409, 488400.000, -0.504,-0.406,
    488430.000, -0.406,-0.412, 488460.000, -0.448,-0.412, 488490.000, -0.606,-0.410,
    488520.000, -0.813,-0.404, 488550.000, -1.005,-0.401, 488580.000, -1.078,-0.403,
    488610.000, -1.022,-0.404, 488640.000, -1.110,-0.400, 488670.000, -1.251,-0.395,
    488700.000, -1.387,-0.390, 488730.000, -1.354,-0.391, 488760.000, -1.197,-0.390,
    488790.000, -0.946,-0.387, 488820.000, -0.788,-0.379, 488850.000, -0.795,-0.377,
    488880.000, -0.830,-0.379, 488910.000, -0.864,-0.391, 488940.000, -0.816,-0.396,
    488970.000, -0.799,-0.396, 489000.000, -0.833,-0.391, 489030.000, -0.918,-0.390,
    489060.000, -0.976,-0.393, 489090.000, -0.962,-0.398, 489120.000, -0.943,-0.398,
    489150.000, -0.961,-0.394, 489180.000, -0.948,-0.396, 489210.000, -0.809,-0.395,
    489240.000, -0.670,-0.396, 489270.000, -0.586,-0.396, 489300.000, -0.581,-0.398,
    489330.000, -0.534,-0.395, 489360.000, -0.466,-0.402, 489390.000, -0.253,-0.413,
    489420.000, -0.122,-0.420, 489450.000, -0.144,-0.422, 489480.000, -0.339,-0.433,
    489510.000, -0.546,-0.450, 489540.000, -0.683,-0.464, 489570.000, -0.921,-0.480,
    489600.000, -1.244,-0.483, 489630.000, -1.742,-0.500, 489660.000, -2.081,-0.493,
    489690.000, -2.116,-0.528, 489720.000, -1.982,-0.540, 489750.000, -1.991,-0.551,
    489780.000, -2.158,-0.554, 490080.000,-17.714,-4.324, 490110.000,-17.941,-4.321,
    490140.000,-18.259,-4.322, 490170.000,-18.417,-4.331, 490200.000,-18.211,-4.340,
    490230.000,-17.821,-4.336, 490260.000,-17.523,-4.335, 490290.000,-17.407,-4.331,
    490320.000,-17.387,-4.333, 490350.000,-17.302,-4.338, 490380.000,-17.090,-4.343,
    490410.000,-16.902,-4.341, 490440.000,-16.835,-4.338, 490470.000,-16.856,-4.340,
    490500.000,-16.888,-4.347, 490530.000,-16.886,-4.354, 490560.000,-16.933,-4.359,
    490590.000,-17.009,-4.361, 490620.000,-17.093,-4.362, 490650.000,-17.066,-4.364,
    490680.000,-16.985,-4.369, 490710.000,-16.935,-4.378, 490740.000,-17.007,-4.382,
    490770.000,-17.151,-4.385, 490800.000,-17.268,-4.390, 490830.000,-17.289,-4.398,
    490860.000,-17.241,-4.404, 490890.000,-17.193,-4.406, 490920.000,-17.230,-4.409,
    490950.000,-17.261,-4.411, 490980.000,-17.250,-4.416, 491010.000,-17.185,-4.420,
    491040.000,-17.089,-4.421, 491070.000,-17.046,-4.421, 491100.000,-17.057,-4.423,
    491130.000,-17.062,-4.428, 491160.000,-17.036,-4.430, 491190.000,-17.039,-4.433,
    491220.000,-17.058,-4.433, 491250.000,-17.119,-4.437, 491280.000,-17.171,-4.441,
    491310.000,-17.171,-4.447, 491340.000,-17.141,-4.452, 491370.000,-17.122,-4.455,
    491400.000,-17.143,-4.456, 491430.000,-17.157,-4.456, 491460.000,-17.143,-4.461,
    491490.000,-17.098,-4.462, 491520.000,-17.105,-4.464, 491550.000,-17.160,-4.464,
    491580.000,-17.221,-4.463, 491610.000,-17.250,-4.462, 491640.000,-17.196,-4.462,
    491670.000,-17.143,-4.462, 491700.000,-17.101,-4.463, 491730.000,-17.047,-4.462,
    491760.000,-17.004,-4.463, 491790.000,-16.961,-4.463, 491820.000,-16.931,-4.461,
    491850.000,-16.929,-4.458, 491880.000,-16.937,-4.457, 491910.000,-16.945,-4.457,
    491940.000,-16.996,-4.457, 491970.000,-17.078,-4.456, 492000.000,-17.160,-4.455,
    492030.000,-17.219,-4.453, 492060.000,-17.244,-4.452, 492090.000,-17.249,-4.452,
    492120.000,-17.256,-4.452, 492150.000,-17.276,-4.455, 492180.000,-17.289,-4.458,
    492210.000,-17.269,-4.461, 492240.000,-17.238,-4.462, 492270.000,-17.195,-4.463,
    492300.000,-17.184,-4.464, 492330.000,-17.214,-4.466, 492360.000,-17.242,-4.468,
    492390.000,-17.248,-4.471, 492420.000,-17.232,-4.472, 492450.000,-17.195,-4.474,
    492480.000,-17.183,-4.473, 492510.000,-17.217,-4.474, 492540.000,-17.278,-4.476,
    492570.000,-17.346,-4.477, 492600.000,-17.409,-4.478, 492630.000,-17.453,-4.479,
    492660.000,-17.501,-4.482, 492690.000,-17.515,-4.484, 492720.000,-17.487,-4.487,
    492750.000,-17.416,-4.489, 492780.000,-17.336,-4.490, 492810.000,-17.268,-4.492,
    492840.000,-17.229,-4.495, 492870.000,-17.188,-4.498, 492900.000,-17.160,-4.498,
    492930.000,-17.163,-4.500, 492960.000,-17.180,-4.502, 492990.000,-17.206,-4.502
   };

   const unsigned int M2=26;
   //# secs-of-wk WLC:12       GF:P:12   
   double data2[2*M2] = {
      489510.000,  -8.546, 489540.000,   9.683, 489570.000,  -7.921,
      489600.000,  -1.244, 489630.000,  -1.742, 489660.000,  -2.081,
      489690.000,  -2.116, 489720.000,  -1.982, 489750.000,  -1.991,
      489780.000,  -2.158, 489810.000, -11.723, 490080.000, -17.714,
      490110.000, -17.941, 490140.000, -18.259, 490170.000, -18.417,
      490200.000, -18.211, 490230.000, -17.821, 490260.000, -17.523,
      490290.000, -17.407, 490320.000, -17.387, 490350.000, -17.302,
      490380.000, -17.090, 490410.000, -16.902, 490440.000, -16.835,
      490470.000, -16.856, 490500.000, -16.888
   };

   //# RinDump output for file /local/Work/HFGeo/G10/G10_0180.14o.new
   const unsigned int M3=245;
   //# secs-of-wk WLC:12       GF:P:12   
   double data3[3*M3] = {
      522000.000,  0.087, 522030.000, -0.039, 522060.000,  0.045, 522090.000, -0.246,
      522120.000, -0.159, 522150.000, -0.006, 522180.000, -0.214, 522210.000,  0.051,
      522240.000,  0.086, 522270.000, -0.019, 522300.000, -0.128, 522330.000,  0.141,
      522360.000, -0.119, 522390.000, -0.216, 522420.000,  0.033, 522450.000, -0.172,
      522480.000,  0.083, 522510.000,  0.022, 522540.000, -0.037, 522570.000, -0.052,
      522600.000, -0.153, 522630.000,  0.114, 522660.000,  0.325, 522690.000,  0.015,
      522720.000, -0.123, 522750.000, -0.054, 522780.000, -0.007, 522810.000, -0.072,
      522840.000, -0.401, 522870.000, -0.098, 522900.000,  0.017, 522930.000, -0.147,
      522960.000, -0.037, 522990.000,  0.027, 523020.000, -0.096, 523050.000, -0.088,
      523080.000,  0.109, 523110.000, -0.115, 523140.000,  0.146, 523170.000, -0.340,
      523200.000, -0.112, 523230.000,  0.219, 523260.000, -0.161, 523290.000, -0.179,
      523320.000,  0.039, 523350.000,  0.037, 523380.000,  0.175, 523410.000, -0.265,
      523440.000, -0.022, 523470.000,  0.204, 523500.000,  0.243, 523530.000, -0.107,
      523560.000, -0.171, 523590.000,  0.334, 523620.000,  0.084, 523650.000,  0.100,
      523680.000, -0.016, 523710.000,  0.035, 523740.000,  0.010, 523770.000, -0.113,
      523800.000, -0.047, 523830.000,  0.179, 523860.000,  0.171, 523890.000, -0.279,
      523920.000, -0.047, 523950.000,  0.405, 523980.000,  0.325, 524010.000, -0.068,
      524040.000,  0.008, 524070.000, -2.719, 524100.000, -0.015, 524130.000,  0.272,
      524160.000,  0.555, 524190.000,  0.300, 524220.000,  0.506, 524250.000,  0.084,
      524280.000,  0.131, 524310.000, -0.109, 524340.000, -0.376, 524370.000,  0.372,
      524400.000,  0.363, 524430.000,  0.063, 524460.000,  0.449, 524490.000,  0.078,
      524520.000,  0.409, 524550.000,  0.128, 524580.000,  0.120, 524610.000,  0.324,
      524640.000,  0.263, 524670.000,  0.123, 524700.000,  0.526, 524730.000, -0.591,
      524760.000,  0.032, 524790.000,  2.959, 524820.000,  0.738, 524850.000, -0.020,
      524880.000,  0.532, 524910.000,  0.783, 524940.000,  0.320, 524970.000, -0.004,
      525000.000,  0.119, 525030.000, -0.641, 525060.000, -0.005, 525090.000, -0.045,
      525120.000,  0.146, 525150.000,  0.202, 525180.000,  0.015, 525210.000,  0.072,
      525240.000,  0.697, 525270.000,  0.885, 525300.000,  0.329, 525330.000,  0.618,
      525360.000, -0.041, 525390.000,  0.072, 525420.000,  0.946, 525450.000,  0.204,
      525480.000,  0.831, 525510.000,  0.257, 525540.000,  0.274, 525570.000,  0.612,
      525600.000,  0.123, 525630.000,  0.107, 525660.000,  0.896, 525690.000,  0.083,
      525720.000,  0.406, 525750.000,  0.398, 525780.000,  0.558, 525810.000,  0.577,
      525840.000, -0.286, 525870.000,  0.226, 525900.000,  0.762, 525930.000,  0.873,
      525960.000,  0.767, 525990.000,  0.865, 526020.000,  0.631, 526050.000,  0.771,
      526080.000,  0.828, 526110.000, -0.308, 526140.000,  0.930, 526170.000,  0.817,
      526200.000,  0.791, 526230.000,  0.048, 526260.000,  0.106, 526290.000,  0.249,
      526320.000,  0.602, 526350.000,  0.099, 526380.000,  0.031, 526410.000,  0.145,
      526440.000,  0.290, 526470.000,  0.635, 526500.000,  1.352, 526530.000,  1.048,
      526560.000,  0.907, 526590.000,  0.743, 526620.000,  0.453, 526650.000,  0.860,
      526680.000,  0.910, 526710.000,  0.483, 526740.000,  0.905, 526770.000,  0.535,
      526800.000, -0.049, 526830.000,  0.308, 526860.000,  1.017, 526890.000, -0.278,
      526920.000,  0.905, 526950.000,  0.373, 526980.000,  0.783, 527010.000, -0.020,
      527040.000,  0.919, 527070.000,  0.503, 527100.000,  1.054, 527130.000,  0.775,
      527160.000,  0.782, 527190.000,  0.236, 527220.000,  0.520, 527250.000,  0.641,
      527280.000,  0.764, 527310.000,  0.264, 527340.000,  1.002, 527370.000,  0.764,
      527400.000,  0.253, 527430.000,  1.204, 527460.000,  0.206, 527490.000,  1.109,
      527520.000,  1.026, 527550.000,  0.501, 527580.000,  1.184, 527610.000, -0.042,
      527640.000,  1.028, 527670.000,  1.123, 527700.000,  2.846, 527730.000,  1.678,
      527760.000,  0.873, 527790.000,  0.908, 527820.000,  2.337, 527850.000, -0.664,
      527880.000,  0.653, 527910.000,  0.200, 527940.000,  0.849, 527970.000,  0.606,
      528000.000, -0.178, 528030.000, -0.046, 528060.000,  0.404, 528090.000,  0.378,
      528120.000,  0.436, 528150.000,  0.146, 528180.000, -0.163, 528210.000,  0.046,
      528240.000, -0.389, 528270.000,  0.566, 528300.000, -0.594, 528330.000, -0.312,
      528360.000, -0.289, 528390.000,  0.694, 528420.000, -0.039, 528450.000,  0.809,
      528480.000,  0.592, 528510.000, -0.687, 528540.000,  1.037, 528570.000,  0.608,
      528600.000,  1.126, 528630.000,  0.736, 528660.000, -0.536, 528690.000,  0.318,
      528720.000,  0.641, 528750.000,  0.223, 528780.000,  0.085, 528810.000,  1.703,
      528840.000,  1.038, 528870.000,  0.572, 528900.000,  0.787, 528930.000, -0.765,
      528960.000, -0.140, 528990.000,  0.712, 529020.000,  0.727, 529050.000,  0.027,
      529080.000,  0.841, 529110.000,  0.568, 529140.000,  0.598, 529170.000, -0.638,
      529200.000,  0.946, 529230.000,  1.173, 529260.000, -0.153, 529290.000,  0.125,
      529320.000,  3.918
   };

   bool verbose(false);             // if true dump all data and results
   int iret,count(0);
   unsigned int i;
   const double tol(0.001);         // for comparing slip size
   string label;
   vector<double> xdata, data, dataB;
   vector< FilterHit<double> > results;

   // dataset 1 ----------------------------------------------------------
   data.clear(); dataB.clear(); xdata.clear();
   for(i=0; i<M1; i++) {
      xdata.push_back(data1[3*i]);
      data.push_back(data1[3*i+1]);
      dataB.push_back(data1[3*i+2]);
   }

   label = "Test1FDF";
   iret = testFirstDiff(xdata, data, 2.0, label, verbose, results);
   if(iret == 0) {
      if(results[0].type != FilterHit<double>::BOD ||
         results[0].npts != 145 || results[0].ngood != 145)
      {
         cout << label << " first hit\n";
         count++;
      }
      if(results[1].type != FilterHit<double>::slip ||
         results[1].index != 145 || results[1].npts != 98 ||
         results[1].ngood != 98 || ::fabs(results[1].step + 15.556) > tol)
      {
         cout << label << " second hit\n";
         count++;
      }
   }
   else { cout << label << " failed " << iret << "\n"; count++; }

   label = "Test1Wind1";
   iret = testWindow(xdata, data, false, 20, 0.08, 6, label, verbose, results);
   if(iret == 239) {
      if(results[0].type != FilterHit<double>::BOD || results[0].index != 2 ||
         results[0].npts != 143 || results[0].ngood != 143)
      {
         cout << label << " first hit\n";
         count++;
      }
      if(results[1].type != FilterHit<double>::slip || results[1].index != 145 ||
         results[1].npts != 96 || results[1].ngood != 96 ||
         ::fabs(results[1].step + 16.379) > tol)
      {
         cout << label << " second hit\n";
         count++;
      }
   }
   else { cout << label << " failed " << iret << "\n"; count++; }

   label = "Test1Wind2";
   iret = testWindow(xdata, data, true, 20, 0.08, 6, label, verbose, results);
   if(iret == 239) {
      if(results[0].type != FilterHit<double>::BOD || results[0].index != 2 ||
         results[0].npts != 143 || results[0].ngood != 143)
      {
         cout << label << " first hit\n";
         count++;
      }
      if(results[1].type != FilterHit<double>::slip || results[1].index != 145 ||
         results[1].npts != 96 || results[1].ngood != 96 ||
         ::fabs(results[1].step + 16.016) > tol)
      {
         cout << label << " second hit\n";
         count++;
      }
   }
   else { cout << label << " failed " << iret << "\n"; count++; }

   label = "Test1Wind3";
   iret = testWindow(xdata, dataB, true, 20, 0.08, 6, label, verbose, results);
   if(iret == 239) {
      if(results[0].type != FilterHit<double>::BOD || results[0].index != 2 ||
         results[0].npts != 50 || results[0].ngood != 50)
      {
         cout << label << " first hit\n";
         count++;
      }
      if(results[1].type != FilterHit<double>::slip || results[1].index != 52 ||
         results[1].npts != 93 || results[1].ngood != 93 ||
         ::fabs(results[1].step + 0.906) > tol)
      {
         cout << label << " second hit\n";
         count++;
      }
      if(results[2].type != FilterHit<double>::slip || results[2].index != 145 ||
         results[2].npts != 96 || results[2].ngood != 96 ||
         ::fabs(results[2].step + 3.720) > tol)
      {
         cout << label << " third hit\n";
         count++;
      }
   }
   else { cout << label << " failed " << iret << "\n"; count++; }

   // dataset 2 ----------------------------------------------------------
   data.clear(); xdata.clear();
   for(i=0; i<M2; i++) {
      xdata.push_back(data2[2*i]);
      data.push_back(data2[2*i+1]);
   }

   label = "Test2FDF";
   iret = testFirstDiff(xdata, data, 2.0, label, verbose, results);
   if(iret == 0) {
      if(results[0].type != FilterHit<double>::outlier ||
         results[0].index != 0 || results[0].npts != 3 || results[0].ngood != 0) {
         count++;
      }
      if(results[1].type != FilterHit<double>::BOD ||
         results[1].index != 3 || results[1].npts != 7 || results[1].ngood != 7) {
         count++;
      }
      if(results[2].type != FilterHit<double>::outlier ||
         results[2].index != 10 || results[2].npts != 1 || results[2].ngood != 0) {
         count++;
      }
      if(results[3].type != FilterHit<double>::slip ||
         results[3].index != 11 || results[3].npts != 15 || results[3].ngood != 15 ||
         ::fabs(results[3].step + 15.556) > tol)
      {
         count++;
      }
   }
   else { cout << label << " failed " << iret << "\n"; count++; }

   label = "Test2Wind";
   iret = testWindow(xdata, data, false, 10, 0.08, 6, label, verbose, results);
   if(iret == 22) {
      if(results[0].type != FilterHit<double>::BOD || results[0].index != 2 ||
         results[0].npts != 22 || results[0].ngood != 22)
      {
         cout << label << " first hit\n";
         count++;
      }
   }
   else { cout << label << " failed " << iret << "\n"; count++; }

   // dataset 3 ----------------------------------------------------------
   data.clear(); xdata.clear();
   for(i=0; i<M3; i++) {
      xdata.push_back(data3[2*i]);
      data.push_back(data3[2*i+1]);
   }

   label = "Test3FDF";
   iret = testFirstDiff(xdata, data, 2.5, label, verbose, results);
   if(iret == 0) {
      if(results[0].type != FilterHit<double>::BOD ||
         results[0].index != 0 ||
         results[0].npts != 69 ||
         results[0].ngood != 69)
            { cout << label << " first hit\n"; count++; }
      if(results[1].type != FilterHit<double>::outlier ||
         results[1].index != 69 ||
         results[1].npts != 1 ||
         results[1].ngood != 0)
            { cout << label << " second hit\n"; count++; }
      if(results[2].type != FilterHit<double>::BOD ||
         results[2].index != 70 ||
         results[2].npts != 23 ||
         results[2].ngood != 23)
            { cout << label << " third hit\n"; count++; }
      if(results[3].type != FilterHit<double>::slip ||
         results[3].index != 93 ||
         results[3].npts != 102 ||
         results[3].ngood != 102 ||
         ::fabs(results[3].step - 2.927) > tol)
            { cout << label << " fourth hit\n"; count++; }
      if(results[4].type != FilterHit<double>::slip ||
         results[4].index != 195 ||
         results[4].npts != 49 ||
         results[4].ngood != 49 ||
         ::fabs(results[4].step + 3.001) > tol)
            { cout << label << " fifth hit\n"; count++; }
      if(results[5].type != FilterHit<double>::outlier ||
         results[5].index != 244 ||
         results[5].npts != 1 ||
         results[5].ngood != 0)
            { cout << label << " sixth hit\n"; count++; }
   }

   label = "Test3FDiffF";
   iret = testFDiff(xdata, data, 0.4, label, verbose, results);
   if(iret == 0) {
      if(results[0].type != FilterHit<double>::outlier ||
         results[0].index != 69 ||
         results[0].npts !=  1 ||
         results[0].dx !=  30.0)
            { cout << label << " first hit\n"; count++; }
      if(results[1].type != FilterHit<double>::slip ||
         results[1].index != 79 ||
         results[1].npts != 1 ||
         ::fabs(results[1].dx - 30.0) > 0.001 ||
         ::fabs(results[1].step - 0.910) > 0.001 ||
         ::fabs(results[1].sigma - 0.497) > 0.001)
            { cout << label << " second hit\n"; count++; }
      if(results[2].type != FilterHit<double>::outlier ||
         results[2].index != 91 ||
         results[2].npts !=  5 ||
         results[2].dx !=  150.0)
            { cout << label << " third hit\n"; count++; }
      if(results[3].type != FilterHit<double>::outlier ||
         results[3].index != 110 ||
         results[3].npts !=  7 ||
         results[3].dx !=  210.0)
            { cout << label << " fourth hit\n"; count++; }
      if(results[4].type != FilterHit<double>::outlier ||
         results[4].index != 122 ||
         results[4].npts !=  1 ||
         results[4].dx !=  30.0)
            { cout << label << " fifth hit\n"; count++; }
      if(results[5].type != FilterHit<double>::slip ||
         results[5].index != 128 ||
         results[5].npts != 1 ||
         ::fabs(results[5].dx - 30.0) > 0.001 ||
         ::fabs(results[5].step + 0.930) > 0.001 ||
         ::fabs(results[5].sigma - 0.548) > 0.001)
            { cout << label << " sixth hit\n"; count++; }
      if(results[6].type != FilterHit<double>::outlier ||
         results[6].index != 137 ||
         results[6].npts !=  1 ||
         results[6].dx !=  30.0)
            { cout << label << " seventh hit\n"; count++; }
      if(results[7].type != FilterHit<double>::outlier ||
         results[7].index != 159 ||
         results[7].npts !=  11 ||
         results[7].dx !=  330.0)
            { cout << label << " eighth hit\n"; count++; }
      if(results[8].type != FilterHit<double>::outlier ||
         results[8].index != 177 ||
         results[8].npts !=  23 ||
         results[8].dx !=  690.0)
            { cout << label << " ninth hit\n"; count++; }
      if(results[9].type != FilterHit<double>::outlier ||
         results[9].index != 209 ||
         results[9].npts !=  36 ||
         results[9].dx !=  1050.0)
            { cout << label << " tenth hit\n"; count++; }
      if(results[10].type != FilterHit<double>::slip ||
         results[10].index != 117 ||
         results[10].npts != 8 ||
         ::fabs(results[10].dx - 240.0) > 0.001 ||
         ::fabs(results[10].step + 3.216) > 0.001 ||
         ::fabs(results[10].sigma - 1.756) > 0.001)
            { cout << label << " eleventh hit\n"; count++; }
      if(results[11].type != FilterHit<double>::slip ||
         results[11].index != 200 ||
         results[11].npts != 24 ||
         ::fabs(results[11].dx - 720.0) > 0.001 ||
         ::fabs(results[11].step + 5.034) > 0.001 ||
         ::fabs(results[11].sigma - 2.626) > 0.001)
            { cout << label << " twelfth hit\n"; count++; }
   }

   // --------------------------------------------------------------------
   cout << "Error count is " << count << endl;
   return count;

} catch(Exception& e) {
   cout << "Threw " << e.what() << endl;
}
}
