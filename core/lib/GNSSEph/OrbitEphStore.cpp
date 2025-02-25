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

/// @file OrbitEphStore.cpp
/// Class for storing and/or computing position, velocity, and clock data using
/// tables of <SatID, <time, OrbitEph> >, initial and final times and search methods.
/// Note this class may be used as is, using OrbitEph rather than its descendants,
/// without health or accuracy information; however most likely the user will
/// define a class that inherits this and uses one the classes derived from OrbitEph,
/// for example GPSEphemerisStore and GPSEphemeris.

#include <iostream>
#include <fstream>
#include <iomanip>

#include "StringUtils.hpp"
#include "MathBase.hpp"
#include "CivilTime.hpp"
#include "TimeString.hpp"
#include "RinexSatID.hpp"  // for dump

#include "OrbitEphStore.hpp"

using namespace std;
using namespace gpstk::StringUtils;

namespace gpstk
{
   //---------------------------------------------------------------------------------
   Xvt OrbitEphStore::getXvt(const SatID& sat, const CommonTime& t) const
   {
      try {
         // get the appropriate OrbitEph
         const OrbitEph *eph = findOrbitEph(sat,t);
         if(!eph)
            GPSTK_THROW(InvalidRequest("No OrbitEph for satellite " + asString(sat)));

         // no consideration is given to health here (OrbitEph does not have health);
         // derived classes should override isHealthy()
         if(onlyHealthy && !eph->isHealthy())
            GPSTK_THROW(InvalidRequest("Not healthy"));

         // compute the position, velocity and time
         Xvt sv = eph->svXvt(t);
         return sv;
      }
      catch(InvalidRequest& ir) { GPSTK_RETHROW(ir); }
   }

   //---------------------------------------------------------------------------------
   void OrbitEphStore::dump(ostream& os, short detail) const
   {
      SatTableMap::const_iterator it;

      os << "Dump of " << getName() << " (detail level=" << detail << "):\n";

      // all detail levels: Overall time limits and size
      os << " BCE table for all satellites has " << size() << " entries;"
         << " Time span is " << (initialTime == CommonTime::END_OF_TIME
                                   ? "End_time" : printTime(initialTime,fmt))
         << " to " << (finalTime == CommonTime::BEGINNING_OF_TIME
                                   ? "Begin_time" : printTime(finalTime,fmt))
         << endl;
      os << " Search method is " << (strictMethod ? "User" : "Past") << endl;

      // detail 0: done
      if(detail == 0)
         ;
      // detail 1: One line per satellite with size and time limits
      else if(detail == 1) {
         for(it = satTables.begin(); it != satTables.end(); it++) {
            const TimeOrbitEphTable& table = it->second;
            CommonTime beg(getInitialTime(it->first));
            CommonTime end(getFinalTime(it->first));
            os << "Sat " << RinexSatID(it->first)
               << " has " << setw(3) << table.size() << " entries;"
               << " Time span is " << (beg == CommonTime::END_OF_TIME
                                   ? "End_time" : printTime(beg,fmt))
               << " to " << (end == CommonTime::BEGINNING_OF_TIME
                                   ? "Begin_time" : printTime(end,fmt))
               << endl;
         }
      }

      // detail 2: One line per OrbitEph
      else if(detail==2) {
         for(it = satTables.begin(); it != satTables.end(); it++) {
            const TimeOrbitEphTable& table = it->second;
            os << "Sat " << RinexSatID(it->first)
               << " has " << table.size() << " entries, with times from "
               << printTime(getInitialTime(it->first),fmt)
               << " to " << printTime(getFinalTime(it->first),fmt)
               << endl;
            TimeOrbitEphTable::const_iterator ei;

            for(ei = table.begin(); ei != table.end(); ei++) {
               os << "SAT " << setw(2) << RinexSatID(it->first)
                  << " TOE " << printTime(ei->second->ctToe,fmt)
                  << " TOC " << printTime(ei->second->ctToc,fmt)
                  << " KEY " << printTime(ei->first,fmt)
                  << " HRS " << fixed << setprecision(2) << setw(5)
                  << (ei->second->endValid-ei->second->beginValid)/3600.0
                  << endl;
            }
         }
         os << "  End of " << getName() << " data." << endl << endl;
      }

         // In this case the output is
         // key, beginValid,  Toe,   endValid
      else if(detail==3) {
         string tf1 = "%02m/%02d/%02y %02H:%02M:%02S";
         string tf2 = "%02H:%02M:%02S";

         for(it = satTables.begin(); it != satTables.end(); it++) {
            const TimeOrbitEphTable& table = it->second;
            os << "Sat " << RinexSatID(it->first)
               << " has " << table.size() << " entries, with times from "
               << printTime(getInitialTime(it->first),fmt)
               << " to " << printTime(getFinalTime(it->first),fmt)
               << endl;
            TimeOrbitEphTable::const_iterator ei;

            os << "  Sat  MM/DD/YY      Key     Begin       Toe       Toc      End"
               << endl;

            for(ei = table.begin(); ei != table.end(); ei++) {
               os << it->first << "  " << printTime(ei->first,tf1)
                              << "  " << printTime(ei->second->beginValid,tf2)
                              << "  " << printTime(ei->second->ctToe,tf2)
                              << "  " << printTime(ei->second->ctToc,tf2)
                              << "  " << printTime(ei->second->endValid,tf2) << endl;
            }
         }
      }

      // detail != 0,1,2,3 - complete dump
      else {
         for (it = satTables.begin(); it != satTables.end(); it++) {
            const TimeOrbitEphTable& table = it->second;
            os << "Sat " << it->first
               << " has " << table.size() << " entries, with times from "
               << printTime(getInitialTime(it->first),fmt)
               << " to " << printTime(getFinalTime(it->first),fmt)
               << endl;

            TimeOrbitEphTable::const_iterator ei;
            for(ei = table.begin(); ei != table.end(); ei++)
               ei->second->dump(os);
         }
      }

      os << "END Dump of " << getName() << " (detail level=" << detail << ")\n";

   } // end OrbitEphStore::dump

   //---------------------------------------------------------------------------------
   // Keeps only one OrbitEph for a given satellite and Toe.
   // If keys are repeated, keep the one with the earliest transmit time.
   OrbitEph* OrbitEphStore::addEphemeris(const OrbitEph* eph)
   {
      OrbitEph *ret(0);
      try {
         // is the satellite found in the table? If not, create one
         if(satTables.find(eph->satID) == satTables.end()) {
            TimeOrbitEphTable newtable;
            satTables[eph->satID] = newtable;
         }

         TimeOrbitEphTable& toet = satTables[eph->satID];

            // Determine the time to use as the key.   
            // If strictMethod==true, the key is the beginning of validity (earliest transmit). 
            // If strictMethod==false, the key is the t-sub-oe. 
         CommonTime keyVal = eph->beginValid;
         if (!strictMethod)
            keyVal = eph->ctToe;

            // if map is empty, load object and return
         if(toet.size() == 0) {
            ret = eph->clone();
            toet[keyVal] = ret;
            updateTimeLimits(ret);
            return ret;
         }

         // Search for beginValid in current keys.
         // If found candidate, should be same data
         // as already in table. Test this by comparing Toe values.
         TimeOrbitEphTable::iterator it = toet.find(keyVal);
         if(it != toet.end()) {
            // is a duplicate found in the table?
            if(it->second->ctToe == eph->ctToe) {
               message = string("duplicate Toe");
               return ret;
            }
            else {
               // Found matching beginValid but different Toe - This shouldn't happen
               string str = "Unexpected matching beginValid time but not Toe, for "
                  + asString(eph->satID)
                  + ", beginValid= " + printTime(eph->beginValid,fmt)
                  + ", Toe(map)= " + printTime(it->second->ctToe,fmt)
                  + ", Toe(candidate)= "+ printTime(eph->ctToe,fmt);
               InvalidParameter ir(str);
               GPSTK_THROW(ir);
            }
         }

         // Did not find eph->beginValid in map
         // N.B:: lower_bound will return element beyond key since there is no match
         it = toet.lower_bound(keyVal);

         if(it==toet.begin()) {
            // candidate is before beginning of map
            if(it->second->ctToe == eph->ctToe) {
               toet.erase(it);
            }
            ret = eph->clone();
            toet[keyVal] = ret;
            updateTimeLimits(ret);
            return ret;
         }

         if(it==toet.end()) {
            // candidate is after end of current map
            // get last item in map and check Toe
            TimeOrbitEphTable::reverse_iterator rit = toet.rbegin();
            if(rit->second->ctToe != eph->ctToe) {
               ret = eph->clone();
               toet[keyVal] = ret;
               updateTimeLimits(ret);
            }
            else message = string("Toe matches last");
            return ret;
         }

         // candidate is "In the middle"
         // Check if iterator points to late transmission of
         // same OrbitEph as candidate
         if(it->second->ctToe == eph->ctToe) {
            toet.erase(it);
            ret = eph->clone();
            toet[keyVal] = ret;
            updateTimeLimits(ret);
            return ret;
         }

         // Two cases:
         //    (a.) Candidate is late transmit copy of
         //         previous OrbitEph in table - discard (do nothing)
         //    (b.) Candidate OrbitEph is not in table
         // Already checked for it==toet.beginValid() earlier
         it--;
         if(it->second->ctToe != eph->ctToe) {
            ret = eph->clone();
            toet[keyVal] = ret;
            updateTimeLimits(ret);
         }
         else message = string("Late transmit copy");
         return ret;
      }
      catch(Exception& e) { GPSTK_RETHROW(e) }

   }  // end OrbitEph* OrbitEphStore::addEphemeris(const OrbitEph* eph)

   //---------------------------------------------------------------------------------
   void OrbitEphStore::edit(const CommonTime& tmin, const CommonTime& tmax)
   {
      for(SatTableMap::iterator i = satTables.begin(); i != satTables.end(); i++)
      {
         TimeOrbitEphTable& eMap = i->second;

         TimeOrbitEphTable::iterator lower = eMap.lower_bound(tmin);
         if(lower != eMap.begin())
         {
            for (TimeOrbitEphTable::iterator emi = eMap.begin(); emi != lower; emi++)
               delete emi->second;
            eMap.erase(eMap.begin(), lower);
         }

         TimeOrbitEphTable::iterator upper = eMap.upper_bound(tmax);
         if(upper != eMap.end())
         {
            for (TimeOrbitEphTable::iterator emi = upper; emi != eMap.end(); emi++)
               delete emi->second;
            eMap.erase(upper, eMap.end());
         }
      }

      initialTime = tmin;
      finalTime   = tmax;
   }

   //---------------------------------------------------------------------------------
   void OrbitEphStore::clear(void)
   {
      for(SatTableMap::iterator ui=satTables.begin(); ui!=satTables.end(); ui++) {
         TimeOrbitEphTable& toet = ui->second;
         for(TimeOrbitEphTable::iterator toeti = toet.begin(); toeti != toet.end(); toeti++) {
            delete toeti->second;
         }
         toet.clear();
      }

      satTables.clear();

      initialTime = CommonTime::END_OF_TIME;
      initialTime.setTimeSystem(timeSystem);
      finalTime = CommonTime::BEGINNING_OF_TIME;
      finalTime.setTimeSystem(timeSystem);
   }

   //---------------------------------------------------------------------------------
   unsigned OrbitEphStore::size(void) const
   {
      unsigned counter = 0;
      SatTableMap::const_iterator it;
      for(it = satTables.begin(); it != satTables.end(); it++)
         counter += it->second.size();
      return counter;
   }

   //---------------------------------------------------------------------------------
   unsigned OrbitEphStore::size(const SatID& sat) const
   {
      unsigned n(0);
      SatTableMap::const_iterator it;
      if(sat.id == -1) {
         for(it = satTables.begin(); it != satTables.end(); ++it) {
            if(it->first.system != sat.system && sat.system != SatID::systemMixed)
               continue;
            n += it->second.size();
         }
      }
      else {
         it = satTables.find(sat);
         if(it == satTables.end()) return 0;
         n = it->second.size();
      }
      return n;
   }

   //---------------------------------------------------------------------------------
   // The goal of this routine is to find the set of orbital elements that would have
   // been used by a receiver in real-time. That is to say, the most recently
   // broadcast elements (assuming receiver has visibility to the given satellite).
   // @return a pointer to the desired OrbitEph, or NULL if no OrbitEph found.
   const OrbitEph* OrbitEphStore::findUserOrbitEph(const SatID& sat,
                                                   const CommonTime& t) const
   {
      // Is this satellite found in the table?
      if(satTables.find(sat) == satTables.end())
         return NULL;

      // Define reference to the relevant map of orbital elements
      const TimeOrbitEphTable& table = getTimeOrbitEphMap(sat);

      // The map is ordered by beginning times of validity, which
      // is another way of saying "earliest transmit time".  A call
      // to table.lower_bound(t) will return the element of the map
      // with a key "just beyond t" assuming the t is NOT a direct match for any key.

      if (table.empty())
         return NULL;

      TimeOrbitEphTable::const_iterator it = table.find(t);
      if(it == table.end()) {                   // not a direct match
         it = table.lower_bound(t);

         // Tricky case here.  If the key is beyond the last key in the table,
         // lower_bound() will return table.end(). However, this doesn't entirely
         // settle the matter. It is theoretically possible that the final
         // item in the table may have an effectivity that "stretches" far enough
         // to cover time t. Therefore, if it==table.end() we need to check
         // the period of validity of the final element in the table against time t.
         if(it == table.end()) {
            TimeOrbitEphTable::const_reverse_iterator rit = table.rbegin();
            if(rit->second->isValid(t))         // Last element in map works
               return rit->second;

            // have nothing
            //string mess = "Time is beyond table for satellite " + asString(sat)
            //   + " for time " + printTime(t,fmt);
            //InvalidRequest e(mess);
            //GPSTK_THROW(e);
            return NULL;
         }
      }  // end if not a direct match

      // Found a direct match. should probably use the PRIOR set
      // since it takes ~30 seconds from beginning of transmission to complete
      // reception.
      // If lower_bound( ) was called, it points to the element after the time t,
      // So either way, it points ONE BEYOND the element we want.
      // The exception is if it is pointing to table.begin( ),
      // then all of the elements in the map are too late.
      if(it == table.begin()) {
         if (it->second->isValid(t))
            return it->second;
         //string mess = "Time is before table for satellite " + asString(sat)
         //      + " for time " + printTime(t,fmt);
         //InvalidRequest e(mess);
         //GPSTK_THROW(e);
         return NULL;
      }

      // The iterator should be a valid iterator and set one beyond
      // the item of interest. However, there may be gaps in the
      // middle of the map and cases where periods of effectivity do
      // not overlap. That's OK, the key represents the EARLIEST
      // time the elements should be used.  Therefore, we can
      // decrement the counter and test to see if the element is valid.
      if (it->second->isValid(t))
      {
         return it->second;
      }
      it--;
      if(!(it->second->isValid(t))) {
         //// there is a "hole" in the middle of a map.
         //string mess = "No orbital elements found for satellite " + asString(sat)
         //   + " at time " + printTime(t,fmt);
         //InvalidRequest e(mess);
         //GPSTK_THROW(e);
         return NULL;
      }

      return it->second;

   }  // end OrbitEph* OrbitEphStore::findUserOrbitEph


   //---------------------------------------------------------------------------------
   const OrbitEph* OrbitEphStore::findNearOrbitEph(const SatID& sat,
                                                   const CommonTime& t) const
   {

        // Check for any OrbitEph for this SV
      if(satTables.find(sat) == satTables.end())
         return NULL;

      // No OrbitEph in store for requested sat time
      // Define reference to the relevant map of orbital elements
      const TimeOrbitEphTable& table = getTimeOrbitEphMap(sat);

      if (table.empty())
         return NULL;

      TimeOrbitEphTable::const_iterator itNext = table.find(t);
      if(itNext != table.end())               // exact match
         return itNext->second;

      // Three cases:
      // 1. t is within a gap within the store
      // 2. t is before all OrbitEph in the store
      // 3. t is after all OrbitEph in the store

      // lower_bound returns the first element with key >= t
      itNext = table.lower_bound(t);
      if(itNext == table.begin())             // Test for case 2
      {
            // Verify the first item in the table has a fit interval that
            // covers the time of interest.   If not, then there are no
            // data sets available that cover the time of interest, so return
            // NULL. 
         if (itNext->second->isValid(t))
            return itNext->second;
         else 
            return NULL;
      }

       // Test for case 3
      if(itNext == table.end()) 
      {
         TimeOrbitEphTable::const_reverse_iterator rit = table.rbegin();
            // Verify the last item in the table has a fit interval that
            // covers the time of interest.   If not, then there are no
            // data sets available that cover the time of interest, so return
            // NULL. 
         if (rit->second->isValid(t))
            return rit->second;
         else
            return NULL; 
      }

      // case 1: it is not the beginning, so safe to decrement
      CommonTime nextTOE = itNext->second->ctToe;
      TimeOrbitEphTable::const_iterator itPrior = itNext;
      itPrior--;
      CommonTime lastTOE = itPrior->second->ctToe;
      double diffToNext = nextTOE - t;
      double diffFromLast = t - lastTOE;

         // Determine which is closer to Toe and assign temporary
         // pointers accordingly.  
      TimeOrbitEphTable::const_iterator itSelect;
      TimeOrbitEphTable::const_iterator itUnSelect;
      if(diffToNext > diffFromLast)
      {
         itSelect = itPrior;
         itUnSelect = itNext;
      }
      else
      {
         itSelect = itNext;
         itUnSelect = itPrior;
      }

         // If the selected item is valid return it.
         // If not, check to see if the unselected item is valid; if so return that one. 
         // Otherwise, there is no data set with a valid fit interval. 
      if (itSelect->second->isValid(t))
      {
         return itSelect->second;
      }
      else if (itUnSelect->second->isValid(t))
      {
         return itUnSelect->second;
      }
      return NULL;
   }

   //---------------------------------------------------------------------------------
   // Add all ephemerides to an existing list<OrbitEph>.
   // If SatID sat is given, limit selections to sat's satellite system, plus if
   // sat's id is not -1, limit to sat's id as well.
   // @return the number of ephemerides added.
   int OrbitEphStore::addToList(list<OrbitEph*>& v, SatID sat) const
   {
      int n = 0;
      SatTableMap::const_iterator it;
      for (it = satTables.begin(); it != satTables.end(); it++)
      {
         if(sat.system != SatID::systemUnknown) {
            if(it->first.system != sat.system) continue;
            if(sat.id != -1 && it->first.id != sat.id) continue;
         }

         const TimeOrbitEphTable& em = it->second;
         TimeOrbitEphTable::const_iterator ei;
         for(ei = em.begin(); ei != em.end(); ei++) {
            v.push_back(ei->second->clone());
            n++;
         }
      }
      return n;
   }

   //---------------------------------------------------------------------------------
   const OrbitEphStore::TimeOrbitEphTable&
      OrbitEphStore::getTimeOrbitEphMap(const SatID& sat) const
   {
      SatTableMap::const_iterator it = satTables.find(sat);
      if(it == satTables.end()) {
         InvalidRequest e("No OrbitEph for satellite " + asString(sat));
         GPSTK_THROW(e);
      }
      return it->second;
   }

   //---------------------------------------------------------------------------------
   const string OrbitEphStore::fmt("%Y/%02m/%02d %02H:%02M:%02S %P");

   //---------------------------------------------------------------------------------
   set<SatID> OrbitEphStore::getIndexSet() const
   {
      set<SatID> retSet;
      SatTableMap::const_iterator cit;
      for (cit=satTables.begin();cit!=satTables.end();cit++)
      {
         const SatID& sidr = cit->first;
         retSet.insert(sidr);
      }
      return retSet;
   }

} // namespace
