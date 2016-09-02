#ifndef MCORE_MMD5CHECKSUM_H
#define MCORE_MMD5CHECKSUM_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MMD5Checksum.h

#include <MCORE/MObject.h>

/// MD5 Checksum updater and calculator class.
///
class M_CLASS MMD5Checksum : public MObject
{
public: // Static members:

   /// Main function that statically calculates MD5 checksum from the given buffer.
   ///
   static MByteString Calculate(const MByteString& data);

public: // Constructor and destructor:

   /// Construct the class so it can be used to accumulate and update checksum
   ///
   MMD5Checksum()
   {
      Reset();
   }

   /// Destructor
   ///
   ~MMD5Checksum()
   {
   }

public: // Method to work with MD5 accumulator:

   /// Reset checksum calculation, start it over.
   ///
   void Reset();

   /// Update MD5 checksum with the given data
   ///
   void Update(const MByteString& data);

   /// Update MD5 checksum with the data given as pointer and length
   ///
   void UpdateWithBytes(const char* body, unsigned len);

   /// Finalize calculation and return the result
   ///
   MByteString GetResult();

private: // Methods:

   void DoTransform(const Muint8*);

private: // Data:

   // Count of bytes in the MD5 calculator
   //
   Muint32 m_count [ 2 ];

   // Final result
   //
   Muint32 m_md5 [ 4 ];

   // Buffer to store context of the calculation
   //
   Muint8  m_buffer [ 64 ];

   M_DECLARE_CLASS(MD5Checksum)
};

///@}
#endif
