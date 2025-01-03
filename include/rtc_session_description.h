#ifndef LIB_WEBRTC_RTC_SESSION_DESCRIPTION_HXX
#define LIB_WEBRTC_RTC_SESSION_DESCRIPTION_HXX

#include "rtc_types.h"
#include "rtc_sdp_parse_error.h"

namespace libwebrtc {

class RTCSessionDescription : public RefCountInterface {
 public:
  enum SdpType { kOffer = 0, kPrAnswer, kAnswer };

  LIB_WEBRTC_API static scoped_refptr<RTCSessionDescription> Create(
      const string type, const string sdp, RTCSdpParseError* error);

 public:
  virtual const string sdp() const = 0;

  virtual const string type() = 0;

  virtual SdpType GetType() = 0;

  virtual bool ToString(string& out) = 0;

 protected:
  virtual ~RTCSessionDescription() {}
};

}  // namespace libwebrtc

#endif  // LIB_WEBRTC_RTC_SESSION_DESCRIPTION_HXX