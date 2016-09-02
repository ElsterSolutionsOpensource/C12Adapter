#ifndef MCORE_MRANDOMGENERATOR_H
#define MCORE_MRANDOMGENERATOR_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MRandomGenerator.h

#include <MCORE/MObject.h>

/// Cryptographically secure random generator.
///
/// The implementation uses operating system services to offer
/// raw buffers filled with random bytes that can be used in cryptography.
/// 
/// \see \ref MMath::Rand() - not cryptographically secure but much faster
///     random number generator function that returns integer
/// \see \ref MMath::RandomInRange() - not cryptographically secure but much faster
///     random number generator function that returns unsigned integer within a desired range
///
class M_CLASS MRandomGenerator : public MObject
{
public:

   /// Create the object so it can be used further for random generation.
   ///
   /// This constructor delays initialization of the generator to the first use of Generate function.
   /// It will never throw any sort of error.
   ///
   MRandomGenerator()
   :
      #if (M_OS & M_OS_WINDOWS) != 0
         m_crypt(NULL) // this is a pointer on Windows
      #else
         m_fd(-1) // non-negative if open
      #endif
   {
   }

   /// Destroy generator object
   ///
   virtual ~MRandomGenerator() M_NO_THROW;

   /// Generate a row of random bytes of a requested length.
   ///
   /// When called the first time after construction of the object, the method is going to initialize the operating system
   /// random generator service, therefore, the first call can take longer, and it can generate an extended set of errors.
   ///
   /// \param size
   ///     Number of random bytes to generate and return
   ///     Size shall not be zero, and shall be less than INT_MAX, which is approximately two billion, or an exception is thrown.
   ///
   /// \return MByteString - the string of bytes with random values
   ///
   /// \see StaticGenerate - convenience method that opens the generator, generates a sequence, and closes the generator, all in a single call
   ///
   MByteString Generate(unsigned size);

   /// Generate random bytes into a buffer provided.
   ///
   /// When called first time after construction of the object, the method is going to initialize the operating system
   /// random generator service, therefore, the first call can take longer, and it can generate an extended set of errors.
   ///
   /// \param buff
   ///     Buffer into which the numbers shall be generated,
   ///     shall be no less than the size given, or the behavior is undefined.
   /// \param size
   ///     Number of random bytes to generate and return.
   ///     Size shall not be zero, and shall be less than INT_MAX, which is approximately two billion, or an exception is thrown.
   ///
   /// \return MByteString - the string of bytes with random values
   ///
   /// \see Generate - generate a random sequence into MByteString
   /// \see StaticGenerateBuffer - same as \ref StaticGenerate, but do the job into a specified buffer
   ///
   void GenerateBuffer(char* buff, unsigned size);

   /// Convenience method that opens the generator, generates a byte string, and closes the generator, all in a single call.
   ///
   /// It makes sense to use this method if only one single random raw buffer of bytes has to be generated,
   /// otherwise, the method will be doing generator initialization at every call, which is extra time and resource.
   ///
   /// \param size
   ///     Number of random bytes to generate and return.
   ///     Size shall not be zero, and shall be less than INT_MAX, which is approximately two billion, or an exception is thrown.
   ///
   /// \return MByteString - the string of bytes with random values
   ///
   /// \see Generate - non-static method that will be faster if multiple sequences have to be generated
   ///
   /// \since MeteringSDK Version 5.0.0.1230.
   ///
   static MByteString StaticGenerate(unsigned size);

   /// Convenience method that opens the generator, generates a sequence into a given buffer, and closes the generator, all in a single call.
   ///
   /// It makes sense to use this method if only one single random raw buffer of bytes has to be generated,
   /// otherwise, the method will be doing generator initialization at every call, which is extra time and resource.
   ///
   /// \param buff
   ///     Buffer into which the numbers shall be generated,
   ///     shall be no less than the size given, or the behavior is undefined.
   /// \param size
   ///     Number of random bytes to generate and return.
   ///     Size shall not be zero, and shall be less than INT_MAX, which is approximately two billion, or an exception is thrown.
   ///
   /// \see Generate - non-static method that will be faster if multiple sequences have to be generated
   /// \see StaticGenerate - same as this method, but it returns a MByteString
   ///
   /// \since MeteringSDK Version 5.0.0.1230.
   ///
   static void StaticGenerateBuffer(char* buff, unsigned size);

private: // Data:

#if (M_OS & M_OS_WINDOWS) != 0
   HCRYPTPROV m_crypt; // this is a pointer of a hidden type on Windows
#else
   int m_fd; // non-negative if open
#endif

   M_DECLARE_CLASS(RandomGenerator)
};

///@}
#endif
