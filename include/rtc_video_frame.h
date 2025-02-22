/**
 * File provided for Reference Use Only by isoft (c) 2025.
 * Copyright (c) isoft. All rights reserved.
 * 
 */

#ifndef LIB_WEBRTC_RTC_VIDEO_FRAME_HXX
#define LIB_WEBRTC_RTC_VIDEO_FRAME_HXX

#include "rtc_types.h"

namespace libwebrtc {

class RTCVideoFrame : public RefCountInterface {
 public:
  enum VideoRotation {
    kVideoRotation_0 = 0,
    kVideoRotation_90 = 90,
    kVideoRotation_180 = 180,
    kVideoRotation_270 = 270
  };

 public:
  LIB_WEBRTC_API static scoped_refptr<RTCVideoFrame> Create(
      int width, int height);

  LIB_WEBRTC_API static scoped_refptr<RTCVideoFrame> Create(
      int width, int height, const uint8_t* buffer, int length);

  LIB_WEBRTC_API static scoped_refptr<RTCVideoFrame> Create(
      int width, int height, const uint8_t* data_y, int stride_y,
      const uint8_t* data_u, int stride_u, const uint8_t* data_v, int stride_v);

  virtual scoped_refptr<RTCVideoFrame> Copy() = 0;

  // The resolution of the frame in pixels. For formats where some planes are
  // subsampled, this is the highest-resolution plane.
  virtual int width() const = 0;
  virtual int height() const = 0;
  virtual int size() const = 0;

  virtual VideoRotation rotation() = 0;

  // Returns pointer to the pixel data for a given plane. The memory is owned by
  // the VideoFrameBuffer object and must not be freed by the caller.
  virtual const uint8_t* DataY() const = 0;
  virtual const uint8_t* DataU() const = 0;
  virtual const uint8_t* DataV() const = 0;

  // Returns the number of bytes between successive rows for a given plane.
  virtual int StrideY() const = 0;
  virtual int StrideU() const = 0;
  virtual int StrideV() const = 0;

  // System monotonic clock, same timebase as rtc::TimeMicros().
  virtual int64_t timestamp_us() const = 0;
  virtual void set_timestamp_us(int64_t timestamp_us) = 0;

  virtual int ConvertToARGB(RTCVideoFrameARGB* pDest) = 0;

  virtual int ScaleFrom(scoped_refptr<RTCVideoFrame> source) = 0;

  virtual int ScaleFrom(RTCVideoFrameARGB* source) = 0;

  virtual int ScaleFrom(RTCVideoFrameYUV* source) = 0;

  virtual int Clear(RTCVideoFrameClearType clearType) = 0;

 protected:
  virtual ~RTCVideoFrame() {}
};

}  // namespace libwebrtc

#endif  // LIB_WEBRTC_RTC_VIDEO_FRAME_HXX
