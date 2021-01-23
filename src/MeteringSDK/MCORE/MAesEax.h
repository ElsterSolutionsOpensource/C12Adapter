#ifndef MCORE_MAESEAX_H
#define MCORE_MAESEAX_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MAesEax.h

#include <MCORE/MAes.h>

/// AES encryption and decryption in EAX mode with authentication.
///
/// Currently only 128-bit AES key is supported, which corresponds to ANSI C12.22 use of EAX mode of AES.
/// Different from all the other MeteringSDK classes that accept AES key as a sequence of hexadecimal characters,
/// this class is primarily using key as a raw byte string exactly 16 bytes long. There is a special
/// extra property \refprop{GetHexKey,HexKey} that does necessary conversion.
///
/// EAX mode can be recommended for all cases where the plain text has
/// entropy lower than cryptographic randomness. The following are features of this mode:
///   - It accepts an extra parameter, clearText, that is used for seeding the cipher.
///     Clear text, or its part, shall be randomly generated. In case of ANSI C12.22,
///     four bytes of initialization vector is random.
///   - It encrypts data of any size, not necessarily divisible by 16.
///   - It provides message authentication through 32-bit MAC number (message authentication code).
///     While this is by far not any type of cryptographic signature,
///     32-bit MAC is good enough for cases when the message has some sort of structure,
///     without which it will not be recognized as valid.
///     Computation of EAX mode MAC is fast, compared to speed of AES.
///
/// There are the following set of methods of class MAesEax, shown in overlapped groups:
///
///   - EAX mode encryption and decryption: EaxEncrypt(), EaxDecrypt(), StaticEaxEncrypt(), StaticEaxDecrypt()
///   - EAX authentication: EaxAuthenticate(), StaticEaxAuthenticate()
///
/// Only one thread shall access this object at a time, however since encryption and decryption are
/// long operations, it is a better design to have a per-thread instance of MAesEax.
///
class M_CLASS MAesEax : public MAes
{
public: // Types:

/// \cond SHOW_INTERNAL
   struct EaxContext
   {
      Muint8 L [ KeySize ] = {};
      Muint8 D [ KeySize ] = {};
      Muint8 Q [ KeySize ] = {};
      ~EaxContext() {
          clear();
      }
      void clear() {
          // erase memory used per security requirment
          memset(&L, 0, KeySize);
          memset(&D, 0, KeySize);
          memset(&Q, 0, KeySize);
      };
   };
/// \endcond

public: // Constructor, destructor:

   /// Create EAX encryption/decryption class without setting the key.
   ///
   /// Any attempt to use this class prior to setting key will fail with an exception.
   ///
   MAesEax();

   /// Create EAX encryption/decryption class with key, given as raw 16 bytes.
   ///
   /// After successful initialization, assuming the key has correct size,
   /// the result object can be used for data encryption or decryption.
   ///
   /// \param key Raw 16 bytes, key to use during encryption or decryption
   ///
   MAesEax(const MByteString& key);

   /// Copy constructor, creates a copy of a given object.
   ///
   /// The key gets copied, if the other object has it set.
   /// If the other object does not have key, the result MAesEax object
   /// will have to be assigned a key before using it for encryption.
   ///
   /// \param other An object from which to create a copy.
   ///
   MAesEax(const MAesEax& other);

   /// Destructor, reclaims memory allocated by the object.
   ///
   virtual ~MAesEax();

public: // Methods:

   /// Assignment operator that copies key from another class.
   ///
   /// \param other Other object from which to copy
   ///
   MAesEax& operator=(const MAesEax& other);

   /// Version of EaxEncrypt that accepts pointers and lengths of the data.
   ///
   /// See \ref EaxEncrypt() for detailed description.
   ///
   /// \return 32-bit MAC, message authentication code of clear text.
   ///
   unsigned EaxEncryptBuffer(const char* clearText, unsigned clearTextSize, char* cipherText, unsigned cipherTextSize);

   /// Performance sensitive version of EaxDecrypt that accepts pointers and lengths of the data.
   ///
   /// See \ref EaxDecrypt() for detailed description.
   ///
   /// \return 32-bit MAC, message authentication code of clear text.
   ///
   unsigned EaxDecryptBuffer(const char* clearText, unsigned clearTextSize, char* cipherText, unsigned cipherTextSize);

   /// Performance sensitive version of EaxAuthenticate that accepts pointers and lengths of the data.
   ///
   /// See \ref EaxAuthenticate() for detailed description.
   ///
   /// \return 32-bit MAC, message authentication code of clear text.
   ///
   unsigned EaxAuthenticateBuffer(const char* clearText, unsigned clearTextSize);

   /// Performance sensitive version of EaxDecrypt that avoids returning a string.
   ///
   /// The method never throws an exception, but rather returns a boolean result.
   /// See \ref EaxDecrypt() for detailed description.
   ///
   /// \return bool - whether the decryption passed MAC authentication.
   ///
   bool EaxDecryptToResult(const MByteString& clearText, const MByteString& data, MByteString& result);

   /// Encrypt and authenticate a given chunk of data with AES using EAX mode.
   ///
   /// EAX mode is capable of handling chunks of data of any size,
   /// it authenticates the message, and it also takes an extra parameter, clearText
   /// that participates in authentication.
   ///
   /// When encryption is to be done once for a given key, \ref StaticEaxEncrypt() call is
   /// more convenient, but when key has to be reused for decryption of many chunks,
   /// using MAesEax::EaxEncrypt() yields better performance when it reuses the same MAesEax instance.
   /// This is because there is a key expansion algorithm that is performed per single key assignment.
   ///
   /// \pre Key shall be set to MAesEax instance, or an exception is thrown.
   ///
   /// \param clearText
   ///      Text that shall be available during EAX decryption with authentication phase.
   ///      Clear text shall be randomly generated, or shall have a randomly generated part.
   ///      In either case the randomly generated part shall have no less than 32 bits of entropy,
   ///      which is the size of MAC, message authentication code, as generated by EaxEncrypt,
   ///      and appended to the end of the returned string.
   ///      Clear text will typically have some open information about the encrypted message.
   ///      Clear text gets authenticated together with plain text.
   ///
   /// \param plainText
   ///      This is the data to be encrypted, the data size is not necessarily divisible by 16.
   ///
   /// \return MByteString - cipher text with 4-byte MAC at the end.
   ///
   /// \see EaxDecrypt - decrypting and authenticating the data encrypted by this method.
   /// \see StaticEaxEncrypt - static version, method of a class that gets key as parameter
   ///
   MByteString EaxEncrypt(const MByteString& clearText, const MByteString& plainText);

   /// Decrypt and authenticate a given chunk of data with AES using EAX mode.
   ///
   /// EAX mode is capable of handling chunks of data of any size,
   /// it authenticates the message, and it also takes an extra parameter, clearText
   /// that participates in authentication.
   ///
   /// When decryption is to be done once for a given key, \ref StaticEaxDecrypt() call is
   /// more convenient, but when key has to be reused for decryption of many chunks,
   /// using MAesEax::EaxDecrypt() yields better performance when it reuses the same MAesEax instance.
   /// This is because there is a key expansion algorithm that is performed per single key assignment.
   ///
   /// \pre Key shall be set to MAesEax instance, or an exception is thrown.
   ///    Also, as the call provides authentication, for the method to succeed,
   ///    the same clearText given at AexEncrypt() shall be given,
   ///    same key shall be set, and cipherText shall be unaltered.
   ///    Otherwise possible tampering is reported as exception.
   ///
   /// \param clearText
   ///      Text that was used during EAX encryption.
   ///      Clear text shall be randomly generated, or shall have a randomly generated part.
   ///      In either case the randomly generated part shall have no less than 32 bits of entropy,
   ///      which is the size of MAC, message authentication code, as generated by EaxEncrypt,
   ///      and appended to the end of the returned string.
   ///      Clear text will typically have some open information about the encrypted message.
   ///      Clear text gets authenticated together with plain text.
   ///
   /// \param cipherText
   ///      This is the data to be decrypted.
   ///      The last four bytes of the data is message authentication code, MAC.
   ///
   /// \return MByteString - result authenticated plain text.
   ///
   /// \see EaxEncrypt - decrypting and authenticating the data decrypted by this method.
   /// \see StaticEaxDecrypt - static version, method of a class that gets key as parameter
   ///
   MByteString EaxDecrypt(const MByteString& clearText, const MByteString& cipherText);

   /// Static version of MAesEax::EaxEncrypt that accepts key as parameter.
   ///
   /// EAX mode is capable of handling chunks of data of any size,
   /// it authenticates the message, and it also takes an extra parameter, clearText
   /// that participates in authentication.
   ///
   /// When encryption is to be done once for a given key, \ref StaticEaxEncrypt() call is
   /// more convenient, but when key has to be reused for decryption of many chunks,
   /// using MAesEax::EaxEncrypt() yields better performance when it reuses the same MAesEax instance.
   /// This is because there is a key expansion algorithm that is performed per single key assignment.
   ///
   /// \param key
   ///    Shall be 16 raw bytes in size, or an exception is thrown.
   ///
   /// \param clearText
   ///      Text that shall be available during EAX decryption with authentication phase.
   ///      Clear text shall be randomly generated, or shall have a randomly generated part.
   ///      In either case the randomly generated part shall have no less than 32 bits of entropy,
   ///      which is the size of MAC, message authentication code, as generated by EaxEncrypt,
   ///      and appended to the end of the returned string.
   ///      Clear text will typically have some open information about the encrypted message.
   ///      Clear text gets authenticated together with plain text.
   ///
   /// \param plainText
   ///      This is the data to be encrypted, the data size is not necessarily divisible by 16.
   ///
   /// \return MByteString - cipher text with 4-byte MAC at the end.
   ///
   /// \see EaxEncrypt - object version of StaticEaxEncrypt
   /// \see StaticEaxDecrypt - static decrypt method, the one that directly corresponds to this encryption method
   ///
   static MByteString StaticEaxEncrypt(const MByteString& key, const MByteString& clearText, const MByteString& plainText);

   /// Static version of MAesEax::EaxDecrypt that accepts key as parameter.
   ///
   /// EAX mode is capable of handling chunks of data of any size,
   /// it authenticates the message, and it also takes an extra parameter, clearText
   /// that participates in authentication.
   ///
   /// When decryption is to be done once for a given key, \ref StaticEaxDecrypt() call is
   /// more convenient, but when key has to be reused for decryption of many chunks,
   /// using MAesEax::EaxDecrypt() yields better performance when it reuses the same MAesEax instance.
   /// This is because there is a key expansion algorithm that is performed per single key assignment.
   ///
   /// \param key
   ///    Key, as used when the message was encrypted with AES in EAX mode,
   ///    shall be exactly 16 bytes in size.
   ///    As the call provides authentication, for the method to succeed,
   ///    the same (correct) key shall be given, the one with which the data were encrypted.
   ///    Otherwise possible tampering is reported as exception.
   ///
   /// \param clearText
   ///      Text that was used during EAX encryption.
   ///      Clear text shall be randomly generated, or shall have a randomly generated part.
   ///      In either case the randomly generated part shall have no less than 32 bits of entropy,
   ///      which is the size of MAC, message authentication code, as generated by EaxEncrypt,
   ///      and appended to the end of the returned string.
   ///      Clear text will typically have some open information about the encrypted message.
   ///      Clear text gets authenticated together with plain text.
   ///
   /// \param cipherText
   ///      This is the data to be decrypted.
   ///      The last four bytes of the data is message authentication code, MAC.
   ///
   /// \return MByteString - result authenticated plain text.
   ///
   /// \see EaxDecrypt - object version of StaticEaxEncrypt
   /// \see StaticEaxEncrypt - static encrypt method, the one that directly corresponds to this decryption method
   ///
   static MByteString StaticEaxDecrypt(const MByteString& key, const MByteString& clearText, const MByteString& cipherText);

   /// Compute MAC of a given message using EAX mode of AES as an algorithm.
   ///
   /// MAC returned is only 32 bits, which is not a cryptographically strong method of message authentication,
   /// however without knowledge of key, a malicious party would have one out of four billion case
   /// when an altered message does pass authentication. This is considered strong enough for cases of ANSI C12.22.
   ///
   /// \pre Key shall be set to MAesEax instance, or an exception is thrown.
   ///
   /// \param clearText
   ///     Message to authenticate
   ///
   /// \return 32-bit message authentication code, MAC.
   ///
   unsigned EaxAuthenticate(const MByteString& clearText);

   /// Static version of MAesEax::EaxAuthenticate that accepts key as parameter.
   ///
   /// MAC returned is only 32 bits, which is not a cryptographically strong method of message authentication,
   /// however without knowledge of key, a malicious party would have one out of four billion case
   /// when an altered message does pass authentication. This is considered strong enough for cases of ANSI C12.22.
   ///
   /// \param key
   ///     Shall be exactly 16 bytes, raw data not hex, or an exception is thrown.
   ///
   /// \param clearText
   ///     Message to authenticate
   ///
   /// \return 32-bit message authentication code, MAC.
   ///
   static unsigned StaticEaxAuthenticate(const MByteString& key, const MByteString& clearText);

private: // Methods:

   // Verify the key size and prepare context for AES in EAX mode. Also calls parent.
   //
   virtual void DoCheckAndPrepareContext();

   virtual void DoDestructContext();

   void DoCTR(const Muint8* ws, Muint8* pn, unsigned sizeN);
   void DoCMAC(Muint8* ws, const Muint8* pN, unsigned SizeN);

private: // Data:

   // Binary key
   //
   MByteString m_key;

   // Context, contains key expansion data
   //
   EaxContext m_eaxContext;

   // Whether key expansion was performed and context set for both AES and EAX
   //
   bool m_contextUpdatedForEax;

   M_DECLARE_CLASS(AesEax)
};

///@}
#endif
