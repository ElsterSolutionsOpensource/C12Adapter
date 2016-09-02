// File MCORE/private/aes_impl_openssl.cxx

#include <openssl/opensslv.h>
#include <openssl/err.h>
#include <openssl/aes.h>
#include <openssl/evp.h>

static void DoConstructAesContext(MAesPrivateContext& context)
{
   context.m_cfd = -1;
}

static void DoDestructAesContext(MAesPrivateContext& context)
{
   if ( context.m_cfd >= 0 )
   {
      ioctl(context.m_cfd, CIOCFSESSION, &context.m_sess.ses);
      close(context.m_cfd);
      context.m_cfd = -1;
      MAes::DestroySecureData(reinterpret_cast<Muint8*>(&context.m_sess), sizeof(context.m_sess));
   }
}

void MAes::DoCheckAndPrepareContext()
{
   if ( m_context.m_cfd < 0 )
   {
      CheckKeySizeValid(m_key);

      m_context.m_cfd = open("/dev/crypto", O_RDWR, 0);
      if ( m_context.m_cfd < 0 )
      {
         MESystemError::ThrowLastSystemError("open(/dev/crypto)");
         M_ENSURED_ASSERT(0);
      }

      // Set close-on-exec (not really needed here)
      if ( fcntl(m_context.m_cfd, F_SETFD, 1) == -1 )
      {
         MESystemError::ThrowLastSystemError("fcntl(F_SETFD)");
         M_ENSURED_ASSERT(0);
      }

      memset(&m_context.m_sess, 0, sizeof(m_context.m_sess));
      m_context.m_sess.cipher = CRYPTO_AES_ECB;
      m_context.m_sess.key = const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(m_key.data()));
      m_context.m_sess.keylen = static_cast<unsigned>(m_key.size());
      if ( ioctl(m_context.m_cfd, CIOCGSESSION, &m_context.m_sess) )
      {
         MESystemError::ThrowLastSystemError("ioctl(CIOCGSESSION)");
         M_ENSURED_ASSERT(0);
      }

#if 0 // debugging only
      struct session_info_op siop;
      siop.ses = m_context.m_sess.ses;
      if ( ioctl(m_context.m_cfd, CIOCGSESSINFO, &siop) )
      {
         MESystemError::ThrowLastSystemError("ioctl(CIOCGSESSINFO)");
         M_ENSURED_ASSERT(0);
      }
      int align = siop.alignmask;
#endif
   }
}

   static void DoCryptodevOp(unsigned operation, MAesPrivateContext& context, const Muint8* from, Muint8* to)
   {
      struct crypt_op op;
      Muint64 buff [ MAes::BlockSize / sizeof(Muint64) ]; // aligned buffer
      memcpy(buff, from, MAes::BlockSize);

      memset(&op, 0, sizeof(op));
      op.len = MAes::BlockSize;
      op.src = reinterpret_cast<unsigned char*>(buff);
      op.dst = reinterpret_cast<unsigned char*>(buff); // same buffer is okay
      op.op = operation;
      op.ses = context.m_sess.ses;
      if ( ioctl(context.m_cfd, CIOCCRYPT, &op) )
      {
         MESystemError::ThrowLastSystemError("ioctl(CIOCCRYPT)");
         M_ENSURED_ASSERT(0);
      }
      memcpy(to, buff, MAes::BlockSize);
   }

void MAes::EncryptBuffer(const Muint8* plainText, Muint8* cipherText)
{
   DoCheckAndPrepareContext();
   DoCryptodevOp(COP_ENCRYPT, m_context, plainText, cipherText);
}

void MAes::DecryptBuffer(const Muint8* cipherText, Muint8* plainText)
{
   DoCheckAndPrepareContext();
   DoCryptodevOp(COP_DECRYPT, m_context, cipherText, plainText);
}

