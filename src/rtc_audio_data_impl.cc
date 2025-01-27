#include "rtc_audio_data_impl.h"
#include "rtc_base/checks.h"
#include "src/internal/audio_util.h"

namespace libwebrtc {

double RTC_FREQUENCY = 1000.0; // Hertz
double RTC_PI = 3.14159265358979323846;

/**
 * class RTCAudioData
 */

scoped_refptr<RTCAudioData> RTCAudioData::Create(
    const uint8_t* data,
    uint32_t bits_per_sample,
    int sample_rate_hz,
    uint32_t num_channels /* = 1 */
) {
    scoped_refptr<RTCAudioData> audio_data =
        scoped_refptr<RTCAudioData>(
            new RefCountedObject<RTCAudioDataImpl>(
                data,
                bits_per_sample,
                sample_rate_hz,
                num_channels
            )
        );
    return audio_data;
}

scoped_refptr<RTCAudioData> RTCAudioData::Create() {
    return RTCAudioData::Create(
        nullptr, // data
        16,      // 16 bits
        44100,   // sample_rate in Hz
        2        // num_channels
    );
}

/**
 * class RTCAudioDataImpl
 */

RTCAudioDataImpl::RTCAudioDataImpl(
    const uint8_t* data,
    uint32_t bits_per_sample,
    int sample_rate_hz,
    uint32_t num_channels
) : bits_per_sample_(bits_per_sample)
  , samples_per_channel_(sample_rate_hz / 100) // for 10 ms chunk
  , sample_rate_hz_(sample_rate_hz)
  , num_channels_(num_channels)
  , total_data_size_(0)
  , data_size_(0)
  , data_(nullptr)
{
  RTC_DCHECK_GE(bits_per_sample_, 16);
  RTC_DCHECK_LE(bits_per_sample_, 32);
  RTC_DCHECK_EQ(bits_per_sample_ % 8, 0);
  RTC_DCHECK_GE(sample_rate_hz_, 16000);
  RTC_DCHECK_GT(num_channels_, 0);
  
  total_data_size_ = data_size_ = samples_per_channel_ * num_channels_ * bits_per_sample_ / 8;
  if (data_size_ > 0) {
    data_ = std::unique_ptr<uint8_t, webrtc::AlignedFreeDeleter>(
        static_cast<uint8_t*>(webrtc::AlignedMalloc(data_size_, 64))
    );
    if (data) {
      memcpy(data_.get(), data, data_size_);
    }
    else {
        memset(data_.get(), 0, data_size_);
    }
  }
}

RTCAudioDataImpl::~RTCAudioDataImpl() {
  data_ = nullptr;
}

int RTCAudioDataImpl::ScaleFrom(scoped_refptr<RTCAudioData> src) {
  if (src == nullptr) {
    return 0;
  }
  
  uint32_t result = 0;
  if (src->data_size() == data_size_ &&
      src->bits_per_sample() == bits_per_sample_ &&
      src->samples_per_channel() == samples_per_channel_ &&
      src->sample_rate_hz() == sample_rate_hz_ &&
      src->num_channels() == num_channels_)
  {
    // direct copy
    memcpy(data_.get(), src->data(), data_size_);
    result = static_cast<uint32_t>(data_size_);
  }
  else
  {
    // scale
    result = AudioScale(
      data_.get(),
      bits_per_sample(),
      samples_per_channel(),
      sample_rate_hz(),
      num_channels(),
      src->data(),
      src->bits_per_sample(),
      src->samples_per_channel(),
      src->sample_rate_hz(),
      src->num_channels()
    );
  }
  return static_cast<int>(result);
}

int RTCAudioDataImpl::Clear(bool fill_1khz_tone /* = false */) {
  if (data_size_ == 0) {
    return 0;
  }

  if (fill_1khz_tone) {
    if (16 == bits_per_sample_) {
      int16_t* pDest = reinterpret_cast<int16_t*>(data_.get());
      for (size_t i = 0; i < samples_per_channel_; ++i) {
        double time = static_cast<double>(i) / sample_rate_hz_;
        int16_t sample = static_cast<int16_t>(32767 * sin(2 * RTC_PI * RTC_FREQUENCY * time));
        for (size_t ch = 0; ch < num_channels_; ++ch) {
            pDest[i * num_channels_ + ch] = sample;
        }
      }
    }
    else if (24 == bits_per_sample_) {
      int32_t* pDest = reinterpret_cast<int32_t*>(data_.get());
      for (size_t i = 0; i < samples_per_channel_; ++i) {
        double time = static_cast<double>(i) / sample_rate_hz_;
        int32_t sample = static_cast<int32_t>(8388607 * sin(2 * RTC_PI * RTC_FREQUENCY * time));
        for (size_t ch = 0; ch < num_channels_; ++ch) {
            uint8_t* pByteDest = reinterpret_cast<uint8_t*>(pDest) + (i * num_channels_ + ch) * 3;
            pByteDest[2] = static_cast<uint8_t>((sample >> 16) & 0xFF);
            pByteDest[1] = static_cast<uint8_t>((sample >> 8) & 0xFF);
            pByteDest[0] = static_cast<uint8_t>(sample & 0xFF);
        }
      }
    }
    else if (32 == bits_per_sample_) {
      int32_t* pDest = reinterpret_cast<int32_t*>(data_.get());
      for (size_t i = 0; i < samples_per_channel_; ++i) {
        double time = static_cast<double>(i) / sample_rate_hz_;
        int32_t sample = static_cast<int32_t>(2147483647 * sin(2 * RTC_PI * RTC_FREQUENCY * time));
        for (size_t ch = 0; ch < num_channels_; ++ch) {
            pDest[i * num_channels_ + ch] = sample;
        }
      }
    }
    else {
      return 0;
    }
  }
  else {
    memset(data_.get(), 0, data_size_);
  }
  
  return static_cast<int>(data_size_);
}

}  // namespace libwebrtc
