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

/**
 * @file EngNav.hpp
 * Engineering units navigation message abstraction.
 */

#ifndef GPSTK_ENGNAV_HPP
#define GPSTK_ENGNAV_HPP


#include <sys/types.h>
#include <ostream>

#include "gpstkplatform.h"
#include "BinUtils.hpp"
#include "GPSWeekSecond.hpp"
#include "NMCTMeta.hpp"

namespace gpstk
{
      /// @ingroup GNSSEph
      //@{

   struct DecodeQuant;

      /**
       * Base class for ICD-GPS-200 navigation messages.  This class
       * provides functions for decoding the bits in navigation
       * message, and is the base class for all "engineering units"
       * types.
       */
   class EngNav
   {
   public:
         /// This enumeration is used by the convertXBit() method.
      enum BitConvertType
      {
         BITS8 = 0,
         BITS10 = 1
      };

         /// default constructor
      EngNav() throw();

         /// destructor
      virtual ~EngNav() {}


         /**
          * Compute and return the parity of the given subframe word,
          * based on the algorihm defined in Section 20.3.5 of
          * IS-GPS-200D.
          * @param sfword The subframe word to compute the parity of.
          * @param psfword The previous word in the subframe (use 0
          *   when sfword is word 1)
          * @param knownUpright When this is set, the data is assumed
          *   to be upright and no D30 inversion is performed
          * @return the 6-bit parity (or, if zeroBits is set, the
          *   6-bit parity along with the two t-bits, the
          *   non-information bits used for parity computation).
          */
      static uint32_t computeParity(uint32_t sfword,
                                    uint32_t psfword,
                                    bool knownUpright=true);

         /**
          * Compute the parity for the given subframe using the prior
          * subframe and a flag to handle the "non-information bits"
          * that appear in certain words of each subframe.
          * @param sfword The subframe word to compute the parity of.
          * @param psfword The previous word in the subframe (use 0
          *   when sfword is word 1)
          * @param nib if true, sfword is one of the words with the
          *   non-information bearing bits (word 2 or 10), and the
          *   parity will be computed as appropriate for that situation.
          * @return sfword with the proper parity bits.
          */
      static uint32_t fixParity(uint32_t sfword,
                                uint32_t psfword,
                                bool nib,
                                bool knownUpright = true);

         /**
          * Perform a parity check on a navigation message subframe.
          * @return true if the parity check is successful.
          */
      static bool checkParity(const uint32_t input[10], bool knownUpright=true);
      static bool checkParity(const std::vector<uint32_t>& v, bool knownUpright=true);


         /// This is the old routine only left around for compatibility
      static bool subframeParity(const long input[10]);


         /// Following two used by checkParity
         /// Get bit 30 from the given subframe word
      static inline uint32_t getd30(uint32_t sfword)
      {
         return (sfword & 0x01);
      }

         /// Get bit 29 from the given subframe word
      static inline uint32_t getd29(uint32_t sfword)
      {
         return ((sfword & 0x02) >> 1);
      }

         /// Get the HOW time from the provided HOW
      static inline unsigned long getHOWTime(uint32_t word2)
      {
         word2 >>= 13;
         word2 &= 0x0001FFFFL;
         return word2 * 6;
      }

         /// Get the TOW count from the provided HOW
      static inline unsigned long getTOW(uint32_t word2)
      {
            // It is assumed that the upper 2 bits of word2 are
            // appropriately zero.  If that's not the case, you have
            // other problems.
         return (word2 >> 13);
      }

         /// Get the subframe ID from the provided HOW
      static inline short getSFID(uint32_t word2)
      {
         word2 >>= 8;
         word2 &= 0x00000007L;
         return word2 ;
      }


         /**
          * Given 10 words of a navigation message subframe (as
          * defined in ICD-GPS-200), convert to the "appropriate" 60
          * FIC floating point values.
          * @param input array of ten 30-bit words (stored in the 30
          * least-significant bits of each long.
          * @param gpsWeek full (>10 bits) GPS week number associated
          * with almanac reference time.
          * @param output 60 FIC floating point values as defined in
          * the documentation for FIC blocks 9 and 62.
          * @return true if successful.
          */
      static bool subframeConvert(const long input[10],
                                  int gpsWeek,
                                  double output[60])
         throw();

         /**
          * Given 10 words of a navigation message subframe (as
          * defined in ICD-GPS-200), convert to the "appropriate" 60
          * FIC floating point values.
          * @param input array of ten 30-bit words (stored in the 30
          * least-significant bits of each long.
          * @param gpsWeek full (>10 bits) GPS week number associated
          * with almanac reference time.
          * @param output 60 FIC floating point values as defined in
          * the documentation for FIC blocks 9 and 62.
          * @return true if successful.
          */
      static bool subframeConvert(const uint32_t input[10],
                                  short gpsWeek,
                                  double output[60])
         throw();

         /** Convert the week number in \c out from 8-bit to full
          * using the full week number \c gpsWeek.
          * @param gpsWeek source full week number.
          * @param out 8-bit week number to convert to full
          * @return true if source and target are within 127 weeks of
          * each other.
          */
      static bool convert8bit(int gpsWeek, double *out)
         throw();

         /** Convert the week number in \c out from 10-bit to full
          * using the full week number \c gpsWeek.
          * @param gpsWeek source full week number.
          * @param out 10-bit week number to convert to full
          * @return true if source and target are within 511 weeks of
          * each other.
          */
      static bool convert10bit(int gpsWeek, double *out)
         throw();

         /** Convert the week number in \c out from 8 or 10-bit to full
          * using the full week number \c fullGPSWeek.
          * @param fullGPSWeek source full week number.
          * @param incompleteGPSWeek week number to convert to full
          * @param type BITS8 (0) or BITS10 (1)
          * @return Full GPS week corresponding to incompleteGPSWeek
          * assuming incompleteGPSWeek is within half the 8/10 bit
          * distance from fullGPSWeek.
          */
      static short convertXBit( short fullGPSWeek,
                                short incompleteGPSWeek,
                                BitConvertType type);

         /**
          * Given a navigation message subframe, return the
          * pattern number to be used in converting the
          * subframe to engineering units.  The patterns are
          * defined in the following table.  The numbers correspond
          * to the ordering of the bit definitions in ICD-GPS-200
          * Figure 20-1.
          *
          * Subframe #   SV_id   Pattern #
          *     1         n/a        1
          *     2         n/a        2
          *     3         n/a        3
          *     4        1-24        4
          *     4          25        5
          *     5          57        6
          *     5       58-62        7
          *     5          56        8
          *     5          63        9
          *     5       52-55       10
          *
          * @param input 10 long integers containing the ten words of
          * the navigation message subframe.
          * @return the pattern ID as defined in the above table.
          */
      static short getSubframePattern(const long input[10])
         throw();
      static short getSubframePattern(const uint32_t input[10])
         throw();

         /**
          * Given an SV/Page ID (1-63), set the subframe ID and page
          * passed to the function.
          * @param svpgid (input) the SV/Page ID to be translated
          *   (typically extracted from subframe 4/5, word 3, bits 3-8
          *   (bit 1=MSB).
          * @param subframe (output) the subframe ID of svpgid (4,5)
          * @param page (output) the page number of svpgid (1-25)
          * @return false if svpgid was invalid
          * @note Some SV/Page IDs are redundant, so the result will
          * be one of the possible pages for that SV/Page ID.
          */
      static bool sv2page(short svpgid, short& subframe, short& page)
         throw();


         /**
          * Given an SV/Page ID (1-63), set the subframe ID and page
          * passed to the function.
          * @param subframe (input) the subframe ID of svpgid (4,5)
          * @param page (input) the page number of svpgid (1-25)
          * @param svpgid (output) the SV/Page ID translated from subframe/page
          * @return false if svpgid was invalid
          * @note Some SV/Page IDs are redundant, so the result will
          * be one of the possible pages for that SV/Page ID.
          */
      static bool sfpage2svid(short subframe, short page, short& svpgid)
         throw();


         /**
          * Given a Z-count representing the TOW in the HOW of a
          * navigation subframe (i.e. TOW * 4), set the subframe ID
          * and page passed to the function.
          * @param zcount (input) the TOW count, in z-counts, of a nav subframe
          * @param subframe (output) the subframe ID of svpgid (4,5)
          * @param page (output) the page number of svpgid (1-25)
          * @return false if zcount was invalid
          */
      static bool zcount2page(unsigned long zcount,
                              short& subframe, short& page)
         throw();

         /**
          * Emit human-readable instance data to the specified stream.
          * @param s stream to which data will be written
          */
      static void dump(std::ostream& s = std::cout);

         /**
          * Get the NMCT validity time from subframe 2.  Refer to
          * IS-GPS-200 section 2.3.3.4.4.
          *
          * @param[in] sf2 The 10 words of subframe 2.
          * @param[in] howWeek the GPS full week for the HOW time in sf2.
          * @param[out] aodo Age of data offset in seconds.
          * @param[out] tnmct The NMCT validity time.
          * @param[out] toe Time of ephemeris.
          * @return true if the AODO is something other than 27900,
          *   which indicates that the NMCT is invalid (i.e. a return
          *   value of false means that the other output data is
          *   invalid as is the NMCT itself).
          * @throw InvalidParameter if sf2 is not subframe 2.
          */
      static bool getNMCTValidity(const uint32_t sf2[10],
                                  unsigned   howWeek,
                                  uint32_t   &aodo,
                                  CommonTime &tnmct,
                                  CommonTime &toe,
                                  CommonTime &tot)
         throw(InvalidParameter);

      static bool getNMCTValidity(const uint32_t sf2[10],
                                  unsigned   howWeek,
                                  NMCTMeta   &meta)
         throw(InvalidParameter)
      {
         return getNMCTValidity(sf2, howWeek, meta.aodo, meta.tnmct, meta.toe,
                                meta.tot);
      }

   private:

         /**
          * Given 10 words of a navigation message subframe, in, and a
          * structure, p, defining a particular conversion, perform
          * the conversion and store the results in the appropriate
          * location in the FIC F array, out.
          *
          * @param input words of navigation message subframe.  Each
          * nav message word is in the 30 lsbs of the corresponding
          * input[i].
          *
          * @param output array of 60 double which correspond to the
          * section of a FIC F array.  The converted output will be
          * placed in the output array at the location specified in
          * the conversion specification.
          *
          * @param p pointer to structure defining conversion to be
          * performed.
          */
      static void convertQuant(const uint32_t input[10],
                               double output[60],
                               DecodeQuant *p)
         throw();
   }; // class EngNav

      //@}

} // namespace

#endif
