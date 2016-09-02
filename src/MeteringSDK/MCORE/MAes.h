#ifndef MCORE_MAES_H
#define MCORE_MAES_H
/// \addtogroup MCORE
///@{
/// \file MCORE/MAes.h

#include <MCORE/MObject.h>
#include <MCORE/private/aes_defs.h>

/// AES encryption and decryption class.
///
/// Currently only 128-bit AES key is supported, but the interface is generic to support other sizes.
/// Different from all the other MeteringSDK classes that accept AES key as a sequence of hexadecimal characters,
/// this class is primarily using the key as a raw byte string exactly 16 bytes long. There is a special
/// extra property \refprop{GetHexKey,HexKey} that does the necessary conversion.
///
/// This class implements the simplest possible ECB mode, in which every chunk of 16 bytes gets
/// parameterless translation into cipher using key. ECB mode has known security issues as the 16-byte chunks
/// of data with the same contents will produce the very same 16-byte chunks of cipher.
/// This fact presents to an attacker the unwanted knowledge about the contents of plain text.
/// ECB mode is still usable for cases when the data to encrypt has a good entropy,
/// and never repeats, such as cryptographic hash or a key itself.
///
/// For EAX mode refer to \ref MAesEax There is also EAX-mode authentication.
///
/// Only one thread shall access this object at a time, however since encryption and decryption are
/// long operations, it is a better design to have a per-thread instance of MAes.
///
class M_CLASS MAes : public MObject
{
public: // Types:

   enum
   {
      /// Supported binary key size in bytes of this AES algorithm.
      ///
      /// Currently this is only 16 bytes, which corresponds to 128-bit AES
      ///
      KeySize = 16,

      /// AES encryption block size.
      ///
      /// This is 16 bytes
      ///
      BlockSize = 16,

      /// Extra size added to key material at wrapping
      ///
      KeyWrapEncryptionExtraSize = 8,

      /// Maximum size of key material for key wrap.
      ///
      /// The result encrypted key material will be KeyWrapMinimumSize + KeyWrapEncryptionExtraSize
      ///
      KeyWrapMinimumSize = KeySize,

      /// Maximum size of key material for key wrap
      ///
      /// The result encrypted key material will be KeyWrapMaximumSize + KeyWrapEncryptionExtraSize
      ///
      KeyWrapMaximumSize = 2048
   };

public: // Constructor, destructor:

   /// Create AES encryption class without setting the key.
   ///
   /// Any attempt to use this class prior to setting the key will fail with an exception.
   ///
   MAes();

   /// Create AES encryption class with key, given as raw 16 bytes.
   ///
   /// After successful initialization, assuming the key has correct size,
   /// the result object can be used for data encryption or decryption.
   ///
   /// \param key Raw 16 bytes, key to use during encryption or decryption
   ///
   MAes(const MByteString& key);

   /// Copy constructor, creates a copy of a given object.
   ///
   /// The key gets copied, if the other object has it set.
   /// If the other object does not have key, the result MAes object
   /// will have to be assigned a key before using it for encryption.
   ///
   /// \param other An object from which to create a copy.
   ///
   MAes(const MAes& other);

   /// Destructor, reclaims memory allocated by the object.
   ///
   virtual ~MAes();

public: // Properties:

   ///@{
   /// AES Key to use by the class, binary representation.
   ///
   /// The key has binary form, not hex, which is different from all the other MeteringSDK classes
   /// that accept AES key as a sequence of hexadecimal characters.
   ///
   /// \pre When assigning to this property, the given key must be exactly 16 bytes in size,
   ///               or an invalid size exception will be thrown.
   ///
   /// \seeprop{GetHexKey,HexKey} property for handling key given as a sequence of hexadecimal characters.
   ///
   const MByteString& GetKey() const
   {
      return m_key;
   }
   void SetKey(const MByteString&);
   ///@}

   ///@{
   /// AES Key to use by the class, hexadecimal representation.
   ///
   /// The key has binary form, not hex, which is different from all the other MeteringSDK classes
   /// that accept AES key as a sequence of hexadecimal characters.
   ///
   /// \since MeteringSDK Version 5.0.
   ///
   /// \pre When assigning to this property, the given key must be a proper hexadecimal string
   ///               that evaluates into exactly 16 bytes in size,
   ///               or an invalid size exception will be thrown.
   ///               Hexadecimal string can have blanks for clarity, but such blanks must not split
   ///               pairs of hexadecimal digits that represent the key.
   ///
   /// \seeprop{GetKey,Key} property for handling key given as raw bytes.
   ///
   MStdString GetHexKey() const;
   void SetHexKey(const MStdString&);
   ///@}

public: // Methods:

   /// Assignment operator that copies the key from another class.
   ///
   /// \param other Other object from which to copy
   ///
   MAes& operator=(const MAes& other);

   /// Verify the byte size of a given binary key is exactly 16 bytes.
   ///
   /// An exception is thrown if the given binary key has size other than 16 bytes.
   ///
   /// \param key The key which size is to be checked
   ///
   static void CheckKeySizeValid(const MByteString& key);

   /// Verify HEX representation and byte size of a given HEX key.
   ///
   /// For the call to succeed, the given string shall be a valid sequence of hex digits
   /// that evaluates into 16-byte raw data. Hex digits can have blanks,
   /// however no blank shall split the hex pair.
   ///
   /// Examples of valid HEX key:
   ///   - "000102030405060708090A0B0C0D0E0F"
   ///   - "0102 0304 0506 0708  090A 0B0C 0D0E 0F00"
   ///
   /// \param key The hex representation of key which size is to be checked
   ///
   static void CheckHexKeySizeValid(const MStdString& key);

   ///@{
   /// Encrypt buffer of size equal to block size
   ///
   /// Plain text and cipher text can be the same buffer in which case
   /// the plain text will be encrypted in-place.
   ///
   /// \param plainText    Buffer of size 16 bytes
   /// \param cipherText   Result buffer of size 16 bytes
   ///
   void EncryptBuffer(const Muint8* plainText, Muint8* cipherText);
   void EncryptBuffer(const char* plainText, char* cipherText)
   {
      EncryptBuffer(reinterpret_cast<const Muint8*>(plainText), reinterpret_cast<Muint8*>(cipherText));
   }
   ///@}

   ///@{
   /// Decrypt buffer of size equal to block size
   ///
   /// Cipher text and plain text can be the same buffer in which case
   /// the plain text will be decrypted in-place.
   ///
   /// \param cipherText   Buffer of size 16 bytes, previously encrypted with this key
   /// \param plainText    Result buffer of size 16 bytes
   ///
   void DecryptBuffer(const Muint8* cipherText, Muint8* plainText);
   void DecryptBuffer(const char* cipherText, char* plainText)
   {
      DecryptBuffer(reinterpret_cast<const Muint8*>(cipherText), reinterpret_cast<Muint8*>(plainText));
   }
   ///@}

   /// Encrypt a given chunk of data with AES using plain and simple ECB mode.
   ///
   /// ECB mode has known security issues as the 16-byte chunks of data
   /// with the same contents will produce the same 16-byte chunks of cipher.
   /// This fact presents to an attacker the unwanted knowledge about the contents of plain text.
   /// ECB mode is still usable for cases when the data to encrypt has a good entropy,
   /// and never repeats, such as cryptographic hash.
   ///
   /// For cases other than encrypting random or pseudorandom data, EAX mode of AES is preferred,
   /// see class \ref MAesEax. This mode is also capable of handling data of sizes not divisible by 16.
   ///
   /// When decryption is to be done once for a given key, \ref StaticDecrypt() call is
   /// more convenient, but when key has to be reused for decryption of many chunks,
   /// using MAes::Decrypt() yields better performance when it reuses the same MAes instance.
   /// This is because there is a key expansion algorithm that is performed per single key assignment.
   ///
   /// \pre Key shall be set to MAes instance, or an exception is thrown.
   ///
   /// \param plainText
   ///      Raw bytes of size divisible by 16.
   ///      If the size is not divisible by 16, an exception is thrown.
   ///
   /// \return MByteString - result cipher text, same size as plainText.
   ///
   /// \see Decrypt - decrypting the data encrypted by this method.
   /// \see StaticEncrypt - static version, method of a class that gets key as parameter
   ///
   MByteString Encrypt(const MByteString& plainText);

   /// Decrypt a given chunk of data with AES using plain and simple ECB mode.
   ///
   /// ECB mode has known security issues as the 16-byte chunks of data
   /// with the same contents will produce the same 16-byte chunks of cipher.
   /// This fact presents to an attacker the unwanted knowledge about the contents of plain text.
   /// ECB mode is still usable for cases when the data to encrypt has a good entropy,
   /// and never repeats, such as cryptographic hash.
   ///
   /// For cases other than encrypting random or pseudorandom data, EAX mode of AES is preferred,
   /// see class \ref MAesEax. This mode is also capable of handling data of sizes not divisible by 16.
   ///
   /// When decryption is to be done once for a given key, \ref StaticDecrypt() call is
   /// more convenient, but when key has to be reused for decryption of many chunks,
   /// using MAes::Decrypt() yields better performance when it reuses the same MAes instance.
   /// This is because there is a key expansion algorithm that is performed per single key assignment.
   ///
   /// \pre Key shall be set to MAes instance, or an exception is thrown.
   ///
   /// \param cipherText
   ///      Raw bytes of size divisible by 16.
   ///      If the size is not divisible by 16, an exception is thrown.
   ///
   /// \return MByteString - result plain text, same size as cipherText.
   ///
   /// \see Encrypt - how to encrypt the data using ECB mode.
   /// \see StaticDecrypt - static version, method of a class that gets key as parameter
   ///
   MByteString Decrypt(const MByteString& cipherText);

   /// Static version of Encrypt that accepts key as parameter.
   ///
   /// ECB mode has known security issues as the 16-byte chunks of data
   /// with the same contents will produce the same 16-byte chunks of cipher.
   /// This fact presents to an attacker the unwanted knowledge about the contents of plain text.
   /// ECB mode is still usable for cases when the data to encrypt has a good entropy,
   /// and never repeats, such as cryptographic hash.
   ///
   /// For cases other than encrypting random or pseudorandom data, EAX mode of AES is preferred,
   /// see class \ref MAesEax. This mode is also capable of handling data of sizes not divisible by 16.
   ///
   /// When encryption is to be done once for a given key, \ref StaticEncrypt() call is
   /// more convenient, but when key has to be reused for encryption of many chunks,
   /// using MAes::Encrypt() yields better performance when it reuses the same MAes instance.
   /// This is because there is a key expansion algorithm that is performed per single key assignment.
   ///
   /// \param key Shall be exactly 16 bytes, binary.
   ///            Notice, the difference from all other MeteringSDK classes is that
   ///            Key property of MAes is binary, not a hex string.
   ///
   /// \param plainText
   ///      Raw bytes of size divisible by 16.
   ///      If the size is not divisible by 16, an exception is thrown.
   ///
   /// \return MByteString - result cipher text, same size as plainText.
   ///
   /// \see Encrypt - object version of StaticEncrypt
   /// \see StaticDecrypt - static version, method of a class that gets key as parameter
   ///
   static MByteString StaticEncrypt(const MByteString& key, const MByteString& plainText);

   /// Static version of Decrypt that accepts key as parameter.
   ///
   /// ECB mode has known security issues as the 16-byte chunks of data
   /// with the same contents will produce the same 16-byte chunks of cipher.
   /// This fact presents to an attacker the unwanted knowledge about the contents of plain text.
   /// ECB mode is still usable for cases when the data to encrypt has a good entropy,
   /// and never repeats, such as cryptographic hash.
   ///
   /// For cases other than encrypting random or pseudorandom data, EAX mode of AES is preferred,
   /// see class \ref MAesEax. This mode is also capable of handling data of sizes not divisible by 16.
   ///
   /// When encryption is to be done once for a given key, \ref StaticDecrypt() call is
   /// more convenient, but when key has to be reused for encryption of many chunks,
   /// using MAes::Decrypt() yields better performance when it reuses the same MAes instance.
   /// This is because there is a key expansion algorithm that is performed per single key assignment.
   ///
   /// \param key Shall be exactly 16 bytes, binary.
   ///            Notice, the difference from all other MeteringSDK classes is that
   ///            Key property of MAes is binary, not a hex string.
   ///
   /// \param cipherText
   ///      Raw bytes of size divisible by 16.
   ///      If the size is not divisible by 16, an exception is thrown.
   ///
   /// \return MByteString - result plain text, same size as cipherText.
   ///
   /// \see Decrypt - object version of StaticEncrypt
   /// \see StaticEncrypt - static version, method of a class that gets key as parameter
   ///
   static MByteString StaticDecrypt(const MByteString& key, const MByteString& cipherText);

   ///@{
   /// Encrypt key material with AES key wrap algorithm
   ///
   /// Cipher text and plain text can be the same buffer in which case
   /// the plain text will be decrypted in-place. However remember the size of cipher text buffer
   /// should be bigger by 8 bytes.
   ///
   /// \param keyText     Buffer with key material that has to be wrapped
   /// \param keyTextSize Size of the buffer with key material
   /// \param cipherText  Result cipher text, the buffer should be 8 bytes bigger than keyTextSize
   /// \return keyTextSize + 8 is returned, cipherText buffer size
   ///
   /// \see KeyWrap for complete description of functionality
   ///
   unsigned KeyWrapBuffer(const Muint8* keyText, unsigned keyTextSize, Muint8* cipherText);
   unsigned KeyWrapBuffer(const char* keyText, unsigned keyTextSize, char* cipherText)
   {
      return KeyWrapBuffer(reinterpret_cast<const Muint8*>(keyText), keyTextSize, reinterpret_cast<Muint8*>(cipherText));
   }
   ///@}

   ///@{
   /// Decrypt key material with AES key wrap algorithm
   ///
   /// If the key is not correct an exception will be thrown as the verification will not be successful.
   /// Plain text and cipher text can be the same buffer in which case
   /// the plain text will be decrypted in-place. However remember the size of cipher text buffer
   /// should be bigger by 8 bytes.
   ///
   /// \param cipherText     Encrypted buffer with key material that has to be decrypted and verified
   /// \param cipherTextSize Size of the encrypted buffer
   /// \param keyText        Result cipher text, the buffer should be 8 bytes bigger than keyTextSize
   /// \return keyTextSize - 8 is returned, keyText buffer size
   ///
   /// \see KeyUnwrap for complete description of functionality
   ///
   unsigned KeyUnwrapBuffer(const Muint8* cipherText, unsigned cipherTextSize, Muint8* keyText);
   unsigned KeyUnwrapBuffer(const char* cipherText, unsigned cipherTextSize, char* keyText)
   {
      return KeyUnwrapBuffer(reinterpret_cast<const Muint8*>(cipherText), cipherTextSize, reinterpret_cast<Muint8*>(keyText));
   }
   ///@}

   /// Wrap the given key material, one or more keys, using  RFC 3394 key wrap algorithm.
   ///
   /// The key wrapping algorithm is described in a NIST document
   /// dated 16 November 2001 and titled "AES Key Wrap Specification"
   /// It is later accepted as RFC 3394.
   /// 
   /// The algorithm, as implemented here, is intended to be able
   /// to wrap an arbitrary number of 128-byte blocks (the specification
   /// allows for 64-bit blocks, but this is a simplification).
   ///
   /// Key wrap:
   ///   1. initialize variables A_0 = IV (0xA6A6A6...)
   ///      \code
   ///        for i=1...n, R_i^0 = P_i 
   ///      \endcode
   ///   2. calculate intermediate values
   ///      \code
   ///        for j=0...5
   ///          for i=1...n
   ///             B = AES_K(A | R_i)
   ///             A = MSB_{64}(B) xor t, where t= (n*j)+1
   ///             R_i = LSB_{64}(B)
   ///      \endcode
   ///   3. Output the results
   ///      \code
   ///        C_0 = A
   ///        for i=1...n
   ///          C_i = R_i
   ///      \endcode
   ///
   /// Note that passed data pointer must start with 8 bytes of
   /// padding in front of the actual data, and that the data 
   /// length must be an integral multiple of 8 bytes.  In other
   /// words, if you have two 128-bit (16-byte) keys you need to
   /// wrap, the data pointer must point to a 24-byte buffer (16+8)
   /// with the actual data starting at offset 8.  The code will 
   /// then process the data in place (so it cannot be ROM) and
   /// uses the key encryption key (kek) which can be in ROM.  When
   /// the algorithm completes, the entire buffer must be transmitted
   /// since the 8 prepended bytes are required to assure that the
   /// data has arrived intact.
   ///
   /// \param keys
   ///     One or more keys, should be 16 bytes or more, size divisible by 8
   ///
   /// \return bytes - cipher, which is 8 bytes longer than the keys string given
   ///
   /// \see KeyUnwrap - reverse operation
   /// \see StaticKeyWrap - version that accepts key directly
   ///
   MByteString KeyWrap(const MByteString& keys);

   /// Unwrap the given cipher and produce the original key material, one or more keys.
   ///
   /// The key unwrap function takes a pointer to the full encrypted data
   /// including the encrypted header.
   ///
   /// The return value is true if there was no error (i.e. the prefix 
   /// matches the fixed prefix that we know we started with) or false 
   /// if there was an error.  In the case of error, the data is 
   /// overwritten with zeros to avoid leaking information which might 
   /// be used to guess the key (such as a known ciphertext attack).
   ///
   /// \param cipher
   ///     Input that contains wrapped key material
   ///
   /// \return bytes - keys, the size is 8 bytes shorter than the given cipher
   ///
   /// \see KeyWrap - reverse operation
   /// \see StaticKeyUnwrap - version that accepts key directly
   ///
   MByteString KeyUnwrap(const MByteString& cipher);

   /// Static variant of KeyWrap
   ///
   /// \see KeyWrap - object version that accepts key as property
   /// \see StaticKeyUnwrap - reverse static operation
   ///
   /// \param key
   ///     Encryption key to use in key wrap, should be the same as the one in the unwrap call
   ///
   /// \param keys
   ///     One or more 16-byte keys to wrap
   ///
   /// \return bytes - cipher, which is 8 bytes longer than the keys string given
   /// 
   static MByteString StaticKeyWrap(const MByteString& key, const MByteString& keys);

   /// Static variant of KeyUnwrap
   ///
   /// \see KeyUnwrap - object version that accepts key as property
   /// \see StaticKeyWrap - reverse static operation
   ///
   /// \param key
   ///     Encryption key to use in key unwrap, should be the same as the one in the wrap call
   ///
   /// \param cipher
   ///     Cipher text as produced by keys wrap call
   ///
   /// \return bytes - byte string of keys, which is 8 bytes shorter than the cipher given
   /// 
   static MByteString StaticKeyUnwrap(const MByteString& key, const MByteString& cipher);

   /// Throw an error that tells about a validation problem in the encrypted or authenticated message.
   ///
   /// Validation is supported in EAX mode of AES, as supported by child class \ref MAesEax, and by key unwrap procedures.
   /// The user might decide to throw this error if there is a separate way of validating the encrypted buffer,
   /// such as MD5 checksum.
   ///
   /// The message thrown mentions that the problem can result from tampering.
   ///
   static M_NORETURN_FUNC void ThrowValidationError();

   ///@{
   /// Destroy secure data such as key, password and so on
   ///
   /// The memory area denoted by the data will be filled with zeros.
   /// Only the data are erased, while the array sizes are unchanged.
   ///
   static void DestroySecureData(MByteString& data);
   static void DestroySecureData(MByteStringVector& data);
   static void DestroySecureData(Muint8* data, unsigned size)
   {
      memset(data, '\0', size);
   }
   static void DestroySecureData(char* data, unsigned size)
   {
      DestroySecureData(reinterpret_cast<Muint8*>(data), size);
   }
   ///@}

   ///@{
   /// Assign one secure data such as key or password to another variable
   ///
   /// As the data size of a new buffer can be different
   /// as a safety measure the previous contents of the destination is erased.
   ///
   /// \param destination Where to put the new secure material.
   /// \param source From which value to copy data. The source value does not change.
   ///
   /// \see MoveSecureData(MByteString& destination, MByteString& source)
   /// \see MoveSecureData(MByteStringVector& destination, MByteStringVector& source)
   /// \see SwapSecureData(MByteString& v1, MByteString& v2);
   /// \see SwapSecureData(MByteStringVector& v1, MByteStringVector& v2);
   ///
   static void AssignSecureData(MByteString& destination, const MByteString& source);
   static void AssignSecureData(MByteStringVector& destination, const MByteStringVector& source);
   ///@}

   ///@{
   /// Move one secure data such as key or password into another variable, destroy source value
   ///
   /// As the data size of a new buffer can be different
   /// as a safety measure the previous contents of the destination is erased.
   /// The contents of the source is destroyed after assignment.
   ///
   /// \param destination Where to put the new secure material.
   /// \param source From which value to copy data. The source value is destroyed after assignment.
   ///
   /// \see AssignSecureData(MByteString& destination, const MByteString& source)
   /// \see AssignSecureData(MByteStringVector& destination, const MByteStringVector& source)
   /// \see SwapSecureData(MByteString& v1, MByteString& v2);
   /// \see SwapSecureData(MByteStringVector& v1, MByteStringVector& v2);
   ///
   static void MoveSecureData(MByteString& destination, MByteString& source);
   static void MoveSecureData(MByteStringVector& destination, MByteStringVector& source);
   ///@}

   ///@{
   /// Swap secure data such as key or password with another data
   ///
   /// As the data sizes of these can be different, the data gets erased to make sure
   /// nothing stays in memory after the operation.
   ///
   /// \param v1 Data to be swapped with v2
   /// \param v2 Data to be swapped with v1
   ///
   /// \see AssignSecureData(MByteString& destination, const MByteString& source)
   /// \see AssignSecureData(MByteStringVector& destination, const MByteStringVector& source)
   /// \see MoveSecureData(MByteString& destination, MByteString& source)
   /// \see MoveSecureData(MByteStringVector& destination, MByteStringVector& source)
   ///
   static void SwapSecureData(MByteString& v1, MByteString& v2);
   static void SwapSecureData(MByteStringVector& v1, MByteStringVector& v2);
   ///@}

protected: // Methods:
/// \cond SHOW_INTERNAL

   // Verify the key size and prepare context for AES operation.
   //
   virtual void DoCheckAndPrepareContext();

   virtual void DoDestructContext();

   void DoKeyWrapUnwrapRangeCheck(int minimum, int maximum, unsigned size);

   void DoCheckKeyWrapArgumentSize(unsigned size)
   {
      DoKeyWrapUnwrapRangeCheck(KeyWrapMinimumSize, KeyWrapMaximumSize, size);
   }

   void DoCheckKeyUnwrapArgumentSize(unsigned size)
   {
      DoKeyWrapUnwrapRangeCheck(KeyWrapMinimumSize + KeyWrapEncryptionExtraSize, KeyWrapMaximumSize + KeyWrapEncryptionExtraSize, size);
   }

protected: // Data:

   // Binary key
   //
   MByteString m_key;

   // Context, contains private structures. Hidden type.
   //
   MAesPrivateContext m_context;

/// \endcond

   M_DECLARE_CLASS(Aes)
};

///@}
#endif
