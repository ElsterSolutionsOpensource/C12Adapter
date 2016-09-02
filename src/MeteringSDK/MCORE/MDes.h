#ifndef MCORE_MDES_H
#define MCORE_MDES_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MDes.h

#include <MCORE/MObject.h>

/// DES encryption and decryption class.
///
/// Only 56-bit DES is supported, 64-bit key and data size.
/// By contemporary standards, pure DES is insecure,
/// and should not be used in new code.
///
class M_CLASS MDes : public MObject
{
public: // Methods:

   /// Static method for encrypting the 64-bit byte string.
   ///
   /// \param key Shall be exactly 8 bytes, binary.
   ///            Only 56 bits out of 64 are used, the lower bit of every byte is ignored.
   ///            Notice, the difference from all other MeteringSDK classes is that
   ///            Key of MDes is binary, not a hex string.
   ///
   /// \param plainText
   ///      Raw bytes of size, exactly 8 bytes.
   ///
   /// \return MByteString - result cipher text, 8 bytes.
   ///
   /// \see StaticDecrypt - decryption method
   ///
   static MByteString StaticEncrypt(const MByteString& key, const MByteString& plainText);

   /// Static method for encrypting the 64-bit byte string.
   ///
   /// \param key Shall be exactly 8 bytes, binary.
   ///            Only 56 bits out of 64 are used, the lower bit of every byte is ignored.
   ///            Notice, the difference from all other MeteringSDK classes is that
   ///            Key of MDes is binary, not a hex string.
   ///
   /// \param cipherText
   ///      Raw bytes of size, exactly 8 bytes.
   ///
   /// \return MByteString - result plain text, 8 bytes.
   ///
   /// \see StaticEncrypt - encryption method
   ///
   static MByteString StaticDecrypt(const MByteString& key, const MByteString& cipherText);

   /// Static method for encrypting the 64-bit byte buffer in ECB mode.
   ///
   /// ECB mode is inherently insecure, just as DES with 56-bit key
   ///
   /// \param key Shall be exactly 8 bytes, binary.
   ///            Only 56 bits out of 64 are used, the lower bit of every byte is ignored.
   ///            Notice, the difference from all other MeteringSDK classes is that
   ///            Key of MDes is binary, not a hex string.
   ///
   /// \param plainText
   ///      Raw bytes of size, divisible by 8 bytes.
   ///
   /// \param cipherText
   ///      Result cipher text, divisible by 8 bytes.
   ///
   /// \param size
   ///      Size if plain and cipher texts in bytes, divisible by 8 bytes.
   ///
   /// \see StaticDecryptBuffer - decryption method
   ///
   static void StaticEncryptBuffer(const char* key, const char* plainText, char* cipherText, unsigned size = 8);

   /// Static method for decrypting the 64-bit byte buffer.
   ///
   /// ECB mode is inherently insecure, just as DES with 56-bit key
   ///
   /// \param key Shall be exactly 8 bytes, binary.
   ///            Only 56 bits out of 64 are used, the lower bit of every byte is ignored.
   ///            Notice, the difference from all other MeteringSDK classes is that
   ///            Key of MDes is binary, not a hex string.
   ///
   /// \param cipherText
   ///      Raw bytes of size, divisible by 8 bytes.
   ///
   /// \param plainText
   ///      Raw bytes of size, divisible by 8 bytes.
   ///
   /// \param size
   ///      Size if plain and cipher texts in bytes, divisible by 8 bytes.
   ///
   /// \see StaticEncryptBuffer - encryption method
   ///
   static void StaticDecryptBuffer(const char* key, const char* cipherText, char* plainText, unsigned size = 8);

private:

   M_DECLARE_CLASS(Des)
};

///@}
#endif
