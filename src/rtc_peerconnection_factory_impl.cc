#include "rtc_peerconnection_factory_impl.h"

#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"
#include "api/create_peerconnection_factory.h"
#include "api/media_stream_interface.h"
#include "api/video_codecs/builtin_video_decoder_factory.h"
#include "api/video_codecs/builtin_video_encoder_factory.h"
#include "modules/audio_device/audio_device_impl.h"
#include "rtc_audio_source_impl.h"
#include "rtc_media_stream_impl.h"
#include "rtc_mediaconstraints_impl.h"
#include "rtc_peerconnection_impl.h"
#include "rtc_rtp_capabilities_impl.h"
#include "rtc_video_device_impl.h"
#include "rtc_video_source_impl.h"
#include "rtc_dummy_video_capturer_impl.h"
#include "rtc_dummy_audio_source_impl.h"
#include "src/internal/dummy_capturer.h"
#include "src/internal/dummy_audio_capturer.h"
#if defined(USE_INTEL_MEDIA_SDK)
#include "src/win/mediacapabilities.h"
#include "src/win/msdkvideodecoderfactory.h"
#include "src/win/msdkvideoencoderfactory.h"
#endif
#if defined(WEBRTC_IOS)
#include "engine/sdk/objc/Framework/Classes/videotoolboxvideocodecfactory.h"
#endif
#include <api/task_queue/default_task_queue_factory.h>

namespace libwebrtc {

#if defined(USE_INTEL_MEDIA_SDK)
std::unique_ptr<webrtc::VideoEncoderFactory> CreateIntelVideoEncoderFactory() {
  if (!owt::base::MediaCapabilities::Get()) {
    return webrtc::CreateBuiltinVideoEncoderFactory();
  }
  return std::make_unique<owt::base::MSDKVideoEncoderFactory>();
}

std::unique_ptr<webrtc::VideoDecoderFactory> CreateIntelVideoDecoderFactory() {
  if (!owt::base::MediaCapabilities::Get()) {
    return webrtc::CreateBuiltinVideoDecoderFactory();
  }
  return std::make_unique<owt::base::MSDKVideoDecoderFactory>();
}
#endif

RTCPeerConnectionFactoryImpl::RTCPeerConnectionFactoryImpl() {}

RTCPeerConnectionFactoryImpl::~RTCPeerConnectionFactoryImpl() {}

bool RTCPeerConnectionFactoryImpl::Initialize(bool use_dummy_audio /*= false*/) {
  use_dummy_audio_ = use_dummy_audio;
  worker_thread_ = rtc::Thread::Create();
  worker_thread_->SetName("worker_thread", nullptr);
  RTC_CHECK(worker_thread_->Start()) << "Failed to start thread";

  signaling_thread_ = rtc::Thread::Create();
  signaling_thread_->SetName("signaling_thread", nullptr);
  RTC_CHECK(signaling_thread_->Start()) << "Failed to start thread";

  network_thread_ = rtc::Thread::CreateWithSocketServer();
  network_thread_->SetName("network_thread", nullptr);
  RTC_CHECK(network_thread_->Start()) << "Failed to start thread";
  if (!audio_device_module_) {
    task_queue_factory_ = webrtc::CreateDefaultTaskQueueFactory();
    worker_thread_->BlockingCall([=] { CreateAudioDeviceModule_w(); });
  }

  if (!rtc_peerconnection_factory_) {
    rtc_peerconnection_factory_ = webrtc::CreatePeerConnectionFactory(
        network_thread_.get(), worker_thread_.get(), signaling_thread_.get(),
        audio_device_module_, webrtc::CreateBuiltinAudioEncoderFactory(),
        webrtc::CreateBuiltinAudioDecoderFactory(),
#if defined(USE_INTEL_MEDIA_SDK)
        CreateIntelVideoEncoderFactory(), CreateIntelVideoDecoderFactory(),
#else
        webrtc::CreateBuiltinVideoEncoderFactory(),
        webrtc::CreateBuiltinVideoDecoderFactory(),
#endif
        nullptr, nullptr);
  }

  if (!rtc_peerconnection_factory_.get()) {
    Terminate();
    return false;
  }

  return true;
}

bool RTCPeerConnectionFactoryImpl::Terminate() {
  worker_thread_->BlockingCall([&] {
    audio_device_impl_ = nullptr;
    video_device_impl_ = nullptr;
  });
  rtc_peerconnection_factory_ = NULL;
  if (audio_device_module_) {
    worker_thread_->BlockingCall([this] { DestroyAudioDeviceModule_w(); });
  }

  return true;
}

void RTCPeerConnectionFactoryImpl::CreateAudioDeviceModule_w() {
  if (!audio_device_module_)
    audio_device_module_ = webrtc::AudioDeviceModule::Create(
        use_dummy_audio_ 
          ? webrtc::AudioDeviceModule::kDummyAudio
          : webrtc::AudioDeviceModule::kPlatformDefaultAudio,
        task_queue_factory_.get());
}

void RTCPeerConnectionFactoryImpl::DestroyAudioDeviceModule_w() {
  if (audio_device_module_) audio_device_module_ = nullptr;
}

scoped_refptr<RTCPeerConnection> RTCPeerConnectionFactoryImpl::Create(
    const RTCConfiguration& configuration,
    scoped_refptr<RTCMediaConstraints> constraints) {
  scoped_refptr<RTCPeerConnection> peerconnection =
      scoped_refptr<RTCPeerConnectionImpl>(
          new RefCountedObject<RTCPeerConnectionImpl>(
              configuration, constraints, rtc_peerconnection_factory_));
  peerconnections_.push_back(peerconnection);
  return peerconnection;
}

void RTCPeerConnectionFactoryImpl::Delete(
    scoped_refptr<RTCPeerConnection> peerconnection) {
  peerconnections_.erase(
      std::remove_if(
          peerconnections_.begin(), peerconnections_.end(),
          [peerconnection](const scoped_refptr<RTCPeerConnection> pc_) {
            return pc_ == peerconnection;
          }),
      peerconnections_.end());
}

scoped_refptr<RTCAudioDevice> RTCPeerConnectionFactoryImpl::GetAudioDevice() {
  if (!audio_device_module_) {
    worker_thread_->BlockingCall([this] { CreateAudioDeviceModule_w(); });
  }

  if (!audio_device_impl_)
    audio_device_impl_ =
        scoped_refptr<AudioDeviceImpl>(new RefCountedObject<AudioDeviceImpl>(
            audio_device_module_, worker_thread_.get()));

  return audio_device_impl_;
}

scoped_refptr<RTCVideoDevice> RTCPeerConnectionFactoryImpl::GetVideoDevice() {
  if (!video_device_impl_)
    video_device_impl_ = scoped_refptr<RTCVideoDeviceImpl>(
        new RefCountedObject<RTCVideoDeviceImpl>(worker_thread_.get()));

  return video_device_impl_;
}

scoped_refptr<RTCAudioSource> RTCPeerConnectionFactoryImpl::CreateAudioSource(
    const string audio_source_label) {
  rtc::scoped_refptr<webrtc::AudioSourceInterface> rtc_source_track =
      rtc_peerconnection_factory_->CreateAudioSource(cricket::AudioOptions());

  scoped_refptr<RTCAudioSourceImpl> source = scoped_refptr<RTCAudioSourceImpl>(
      new RefCountedObject<RTCAudioSourceImpl>(rtc_source_track));
  return source;
}

scoped_refptr<RTCDummyAudioSource> RTCPeerConnectionFactoryImpl::CreateDummyAudioSource(
    const string audio_source_label,
    int sample_rate_hz /*= 16000 */,
    uint32_t num_channels /*= 2*/
) {

  if (!use_dummy_audio_) {
    return nullptr;
  }

  rtc::scoped_refptr<webrtc::internal::DummyAudioCapturer> rtc_source_track =
      rtc::scoped_refptr<webrtc::internal::DummyAudioCapturer>(
        new rtc::RefCountedObject<webrtc::internal::DummyAudioCapturer>(
          task_queue_factory_.get(),
          signaling_thread_.get(),
          16, /* bits_per_sample */
          sample_rate_hz,
          num_channels,
          audio_source_label.std_string()
        )
      );

  scoped_refptr<RTCDummyAudioSource> source = scoped_refptr<RTCDummyAudioSource>(
      new RefCountedObject<RTCDummyAudioSourceImpl>(rtc_source_track));
  return source;
}

#ifdef RTC_DESKTOP_DEVICE
scoped_refptr<RTCDesktopDevice>
RTCPeerConnectionFactoryImpl::GetDesktopDevice() {
  if (!desktop_device_impl_) {
    desktop_device_impl_ = scoped_refptr<RTCDesktopDeviceImpl>(
        new RefCountedObject<RTCDesktopDeviceImpl>(signaling_thread_.get()));
  }
  return desktop_device_impl_;
}
#endif

scoped_refptr<RTCVideoSource> RTCPeerConnectionFactoryImpl::CreateVideoSource(
    scoped_refptr<RTCVideoCapturer> capturer, const string video_source_label,
    scoped_refptr<RTCMediaConstraints> constraints) {
  if (rtc::Thread::Current() != signaling_thread_.get()) {
    scoped_refptr<RTCVideoSource> source = signaling_thread_->BlockingCall(
        [this, capturer, video_source_label, constraints] {
          return CreateVideoSource_s(
              capturer, to_std_string(video_source_label).c_str(), constraints);
        });
    return source;
  }

  return CreateVideoSource_s(
      capturer, to_std_string(video_source_label).c_str(), constraints);
}

scoped_refptr<RTCVideoSource> RTCPeerConnectionFactoryImpl::CreateVideoSource_s(
    scoped_refptr<RTCVideoCapturer> capturer, const char* video_source_label,
    scoped_refptr<RTCMediaConstraints> constraints) {
  RTCVideoCapturerImpl* capturer_impl =
      static_cast<RTCVideoCapturerImpl*>(capturer.get());
  /*RTCMediaConstraintsImpl* media_constraints =
          static_cast<RTCMediaConstraintsImpl*>(constraints.get());*/
  rtc::scoped_refptr<webrtc::VideoTrackSourceInterface> rtc_source_track =
      rtc::scoped_refptr<webrtc::VideoTrackSourceInterface>(
          new rtc::RefCountedObject<webrtc::internal::CapturerTrackSource>(
              capturer_impl->video_capturer()));
  scoped_refptr<RTCVideoSourceImpl> source = scoped_refptr<RTCVideoSourceImpl>(
      new RefCountedObject<RTCVideoSourceImpl>(rtc_source_track));
  return source;
}

scoped_refptr<RTCVideoSource> RTCPeerConnectionFactoryImpl::CreateDummyVideoSource_s(
      scoped_refptr<RTCDummyVideoCapturer> capturer, const char* video_source_label)
{
  rtc::scoped_refptr<webrtc::VideoTrackSourceInterface> rtc_source_track =
      rtc::scoped_refptr<webrtc::VideoTrackSourceInterface>(
          new rtc::RefCountedObject<DummyCapturerTrackSource>(capturer));

  scoped_refptr<RTCVideoSourceImpl> source = scoped_refptr<RTCVideoSourceImpl>(
      new RefCountedObject<RTCVideoSourceImpl>(rtc_source_track));

  return source;
}

#ifdef RTC_DESKTOP_DEVICE
scoped_refptr<RTCVideoSource> RTCPeerConnectionFactoryImpl::CreateDesktopSource(
    scoped_refptr<RTCDesktopCapturer> capturer, const string video_source_label,
    scoped_refptr<RTCMediaConstraints> constraints) {
  if (rtc::Thread::Current() != signaling_thread_.get()) {
    scoped_refptr<RTCVideoSource> source = signaling_thread_->BlockingCall(
        [this, capturer, video_source_label, constraints] {
          return CreateDesktopSource_d(
              capturer, to_std_string(video_source_label).c_str(), constraints);
        });
    return source;
  }

  return CreateDesktopSource_d(
      capturer, to_std_string(video_source_label).c_str(), constraints);
}

scoped_refptr<RTCVideoSource>
RTCPeerConnectionFactoryImpl::CreateDesktopSource_d(
    scoped_refptr<RTCDesktopCapturer> capturer, const char* video_source_label,
    scoped_refptr<RTCMediaConstraints> constraints) {
  rtc::scoped_refptr<webrtc::VideoTrackSourceInterface> rtc_source_track =
      rtc::scoped_refptr<webrtc::VideoTrackSourceInterface>(
          new rtc::RefCountedObject<ScreenCapturerTrackSource>(capturer));

  scoped_refptr<RTCVideoSourceImpl> source = scoped_refptr<RTCVideoSourceImpl>(
      new RefCountedObject<RTCVideoSourceImpl>(rtc_source_track));

  return source;
}
#endif

scoped_refptr<RTCMediaStream> RTCPeerConnectionFactoryImpl::CreateStream(
    const string stream_id) {
  rtc::scoped_refptr<webrtc::MediaStreamInterface> rtc_stream =
      rtc_peerconnection_factory_->CreateLocalMediaStream(
          to_std_string(stream_id));

  scoped_refptr<MediaStreamImpl> stream = scoped_refptr<MediaStreamImpl>(
      new RefCountedObject<MediaStreamImpl>(rtc_stream));

  return stream;
}

scoped_refptr<RTCVideoTrack> RTCPeerConnectionFactoryImpl::CreateVideoTrack(
    scoped_refptr<RTCVideoSource> source, const string track_id) {
  scoped_refptr<RTCVideoSourceImpl> source_adapter(
      static_cast<RTCVideoSourceImpl*>(source.get()));
  rtc::scoped_refptr<webrtc::VideoTrackInterface> rtc_video_track =
      rtc_peerconnection_factory_->CreateVideoTrack(
          source_adapter->rtc_source_track(), track_id.std_string());

  scoped_refptr<VideoTrackImpl> video_track = scoped_refptr<VideoTrackImpl>(
      new RefCountedObject<VideoTrackImpl>(rtc_video_track));

  // 	webrtc::VideoTrackProxyWithInternal<webrtc::VideoTrackInterface>
  // *track_proxy =
  // dynamic_cast<webrtc::VideoTrackProxyWithInternal<webrtc::VideoTrackInterface>
  // *>(video_track.get()); 	if (track_proxy) {
  // 		webrtc::MediaStreamTrack<VideoTrackInterface> *track =
  // dynamic_cast<webrtc::MediaStreamTrack<VideoTrackInterface>*>(track_proxy->internal());
  // 		LOG(INFO) << "VideoTrackInterface: " << track->id();
  // 	}

  return video_track;
}

scoped_refptr<RTCDummyVideoCapturer> RTCPeerConnectionFactoryImpl::CreateDummyVideoCapturer(
      uint32_t fps, uint32_t width, uint32_t height)
{
  return scoped_refptr<RTCDummyVideoCapturer>(
    new RefCountedObject<RTCDummyVideoCapturerImpl>(
      signaling_thread_.get(),
      fps,
      width,
      height
    )
  );
}

scoped_refptr<RTCVideoSource> RTCPeerConnectionFactoryImpl::CreateDummyVideoSource(
      scoped_refptr<RTCDummyVideoCapturer> capturer, const string video_source_label)
{
  if (rtc::Thread::Current() != signaling_thread_.get()) {
    scoped_refptr<RTCVideoSource> source = signaling_thread_->BlockingCall(
        [this, capturer, video_source_label] {
          return CreateDummyVideoSource_s(
              capturer, to_std_string(video_source_label).c_str());
        });
    return source;
  }

  return CreateDummyVideoSource_s(
      capturer, to_std_string(video_source_label).c_str());
}

scoped_refptr<RTCAudioTrack> RTCPeerConnectionFactoryImpl::CreateAudioTrack(
    scoped_refptr<RTCAudioSource> source, const string track_id) {
  RTCAudioSourceImpl* source_impl =
      static_cast<RTCAudioSourceImpl*>(source.get());

  rtc::scoped_refptr<webrtc::AudioTrackInterface> audio_track(
      rtc_peerconnection_factory_->CreateAudioTrack(
          to_std_string(track_id), source_impl->rtc_audio_source().get()));

  scoped_refptr<AudioTrackImpl> track = scoped_refptr<AudioTrackImpl>(
      new RefCountedObject<AudioTrackImpl>(audio_track));
  return track;
}

scoped_refptr<RTCAudioTrack> RTCPeerConnectionFactoryImpl::CreateAudioTrack(
    scoped_refptr<RTCDummyAudioSource> source, const string track_id) {

  if (!use_dummy_audio_) {
    return nullptr;
  }

  RTCDummyAudioSourceImpl* source_impl =
      static_cast<RTCDummyAudioSourceImpl*>(source.get());

  rtc::scoped_refptr<webrtc::AudioTrackInterface> audio_track(
      rtc_peerconnection_factory_->CreateAudioTrack(
          to_std_string(track_id), source_impl->rtc_audio_source().get()));

  scoped_refptr<AudioTrackImpl> track = scoped_refptr<AudioTrackImpl>(
      new RefCountedObject<AudioTrackImpl>(audio_track));
  return track;
}

scoped_refptr<RTCRtpCapabilities>
RTCPeerConnectionFactoryImpl::GetRtpSenderCapabilities(
    RTCMediaType media_type) {
  if (rtc::Thread::Current() != signaling_thread_.get()) {
    scoped_refptr<RTCRtpCapabilities> capabilities =
        signaling_thread_->BlockingCall([this, media_type] {
          return GetRtpSenderCapabilities(media_type);
        });
    return capabilities;
  }

  cricket::MediaType type = cricket::MediaType::MEDIA_TYPE_AUDIO;
  switch (media_type) {
    case RTCMediaType::AUDIO:
      type = cricket::MediaType::MEDIA_TYPE_AUDIO;
      break;
    case RTCMediaType::VIDEO:
      type = cricket::MediaType::MEDIA_TYPE_VIDEO;
      break;
    default:
      break;
  }
  webrtc::RtpCapabilities rtp_capabilities =
      rtc_peerconnection_factory_->GetRtpSenderCapabilities(type);
  return scoped_refptr<RTCRtpCapabilities>(
      new RefCountedObject<RTCRtpCapabilitiesImpl>(rtp_capabilities));
}

scoped_refptr<RTCRtpCapabilities>
RTCPeerConnectionFactoryImpl::GetRtpReceiverCapabilities(
    RTCMediaType media_type) {
  if (rtc::Thread::Current() != signaling_thread_.get()) {
    scoped_refptr<RTCRtpCapabilities> capabilities =
        signaling_thread_->BlockingCall([this, media_type] {
          return GetRtpSenderCapabilities(media_type);
        });
    return capabilities;
  }
  cricket::MediaType type = cricket::MediaType::MEDIA_TYPE_AUDIO;
  switch (media_type) {
    case RTCMediaType::AUDIO:
      type = cricket::MediaType::MEDIA_TYPE_AUDIO;
      break;
    case RTCMediaType::VIDEO:
      type = cricket::MediaType::MEDIA_TYPE_VIDEO;
      break;
    default:
      break;
  }
  webrtc::RtpCapabilities rtp_capabilities =
      rtc_peerconnection_factory_->GetRtpReceiverCapabilities(type);
  return scoped_refptr<RTCRtpCapabilities>(
      new RefCountedObject<RTCRtpCapabilitiesImpl>(rtp_capabilities));
}

}  // namespace libwebrtc
