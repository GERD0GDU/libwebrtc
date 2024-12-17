#ifndef LIB_WEBRTC_INTROP_API_HXX
#define LIB_WEBRTC_INTROP_API_HXX

#ifdef LIB_WEBRTC_API_EXPORTS
#define LIB_WEBRTC_CALL __stdcall
#elif defined(LIB_WEBRTC_API_DLL)
#define LIB_WEBRTC_CALL __stdcall
#else
#define LIB_WEBRTC_CALL
#endif

#include "rtc_types.h"

#define CHECK_POINTER_EX(p, r)          if ((p) == nullptr) { return (r); }
#define CHECK_POINTER(p)                CHECK_POINTER_EX(p, rtcResultU4::kInvalidPointer)
#define RESET_OUT_POINTER_EX(p,v)       if ((p) != nullptr) { *(p) = (v); }
#define RESET_OUT_POINTER(p)            RESET_OUT_POINTER_EX(p, nullptr)
#define CHECK_OUT_POINTER(p)            if ((p) != nullptr) { *(p) = nullptr; } else { return rtcResultU4::kInvalidPointer; }
#define CHECK_NATIVE_HANDLE(h)          CHECK_POINTER_EX(h, rtcResultU4::kInvalidNativeHandle)
#define ZERO_MEMORY(p,sz)               if ((sz) > 0) { memset((void*)(p), 0, (size_t)(sz)); }

extern "C" {

/// Types used from libwebrtc.
using rtcIceTransportsType = libwebrtc::IceTransportsType;
using rtcBundlePolicy = libwebrtc::BundlePolicy;
using rtcRtcpMuxPolicy = libwebrtc::RtcpMuxPolicy;
using rtcCandidateNetworkPolicy = libwebrtc::CandidateNetworkPolicy;
using rtcTcpCandidatePolicy = libwebrtc::TcpCandidatePolicy;
using rtcMediaSecurityType = libwebrtc::MediaSecurityType;
using rtcSdpSemantics = libwebrtc::SdpSemantics;
using rtcMediaType = libwebrtc::RTCMediaType;
using rtcDesktopType = libwebrtc::DesktopType;

/// 32-bit boolean for interop API.
enum class rtcBool32 : int { kTrue = -1, kFalse = 0 };

enum class rtcTrackState : int { kUnknown = -1, kLive = 0, kEnded = 1 };

/// 32-bit result enumerator
enum class rtcResultU4 : unsigned int {
    /// The operation was successful.
    kSuccess = 0,

    //
    // Generic errors
    //

    /// Unknown internal error.
    /// This is generally the fallback value when no other error code applies.
    kUnknownError = 0x80000000,

    /// A parameter passed to the API function was invalid.
    kInvalidParameter = 0x80000001,

    /// The operation cannot be performed in the current state.
    kInvalidOperation = 0x80000002,

    /// A call was made to an API function on the wrong thread.
    /// This is generally related to platforms with thread affinity like UWP.
    kWrongThread = 0x80000003,

    /// An object was not found.
    kNotFound = 0x80000004,

    /// An interop handle referencing a native object instance is invalid,
    /// although the API function was expecting a valid object.
    kInvalidNativeHandle = 0x80000005,

    /// The API object is not initialized, and cannot as a result perform the
    /// given operation.
    kNotInitialized = 0x80000006,

    /// The current operation is not supported by the implementation.
    kUnsupported = 0x80000007,

    /// An argument was passed to the API function with a value out of the
    /// expected range.
    kOutOfRange = 0x80000008,

    /// The buffer provided by the caller was too small for the operation to
    /// complete successfully.
    kBufferTooSmall = 0x80000009,

    //
    // Peer connection (0x1xx)
    //

    /// The peer connection is closed, but the current operation requires an open
    /// peer connection.
    kPeerConnectionClosed = 0x80000101,

    //
    // Data (0x3xx)
    //

    /// The SCTP handshake for data channels encryption was not performed, because
    /// the connection was established before any data channel was added to it.
    /// Due to limitations in the implementation, without SCTP handshake data
    /// channels cannot be used, and therefor applications expecting to use data
    /// channels must open at least a single channel before establishing a peer
    /// connection (calling |CreateOffer()|).
    kSctpNotNegotiated = 0x80000301,

    /// The specified data channel ID is invalid.
    kInvalidDataChannelId = 0x80000302,

    //
    // Media (0x4xx)
    //

    /// Some audio-only function was called on a video-only object or vice-versa.
    /// For example, trying to get the local audio track of a video transceiver.
    kInvalidMediaKind = 0x80000401,

    /// The internal audio resampler used in the audio track read buffer doesn't
    /// support the specified input/output frequency ratio. Use a different output
    /// frequency for the current audio source to solve the issue.
    kAudioResamplingNotSupported = 0x80000402,

    /// Error rtcResultU4 for a null or invalid pointer.
    kInvalidPointer = 0x80004003,
}; // end enum class rtcResultU4

struct rtcIceServer {
    const char* uri = nullptr;
    const char* username = nullptr;
    const char* password = nullptr;
};

struct rtcPeerConnectionConfiguration {
    rtcIceServer ice_servers[libwebrtc::kMaxIceServerSize];
    rtcIceTransportsType type = rtcIceTransportsType::kAll;
    rtcBundlePolicy bundle_policy = rtcBundlePolicy::kBundlePolicyBalanced;
    rtcRtcpMuxPolicy rtcp_mux_policy = rtcRtcpMuxPolicy::kRtcpMuxPolicyRequire;
    rtcCandidateNetworkPolicy candidate_network_policy =
        rtcCandidateNetworkPolicy::kCandidateNetworkPolicyAll;
    rtcTcpCandidatePolicy tcp_candidate_policy =
        rtcTcpCandidatePolicy::kTcpCandidatePolicyEnabled;

    int ice_candidate_pool_size = 0;

    rtcMediaSecurityType srtp_type = rtcMediaSecurityType::kDTLS_SRTP;
    rtcSdpSemantics sdp_semantics = rtcSdpSemantics::kUnifiedPlan;
    rtcBool32 offer_to_receive_audio = rtcBool32::kTrue;
    rtcBool32 offer_to_receive_video = rtcBool32::kTrue;

    rtcBool32 disable_ipv6 = rtcBool32::kFalse;
    rtcBool32 disable_ipv6_on_wifi = rtcBool32::kFalse;
    int max_ipv6_networks = 5;
    rtcBool32 disable_link_local_networks = rtcBool32::kFalse;
    int screencast_min_bitrate = -1;

    // private
    rtcBool32 use_rtp_mux = rtcBool32::kTrue;
    uint32_t local_audio_bandwidth = 128;
    uint32_t local_video_bandwidth = 512;
};

/// Opaque handle to a native interop object.
using rtcObjectHandle = void*;

/// Opaque handle to a native reference-counted interop object.
using rtcRefCountedObjectHandle = rtcObjectHandle;

/// Opaque handle to a native RTCPeerConnectionFactory interop object.
using rtcPeerConnectionFactoryHandle = rtcRefCountedObjectHandle;

/// Opaque handle to a native RTCPeerConnection interop object.
using rtcPeerConnectionHandle = rtcRefCountedObjectHandle;

/// Opaque handle to a native RTCAudioDevice interop object.
using rtcAudioDeviceHandle = rtcRefCountedObjectHandle;

/// Opaque handle to a native RTCVideoDevice interop object.
using rtcVideoDeviceHandle = rtcRefCountedObjectHandle;

/// Opaque handle to a native RTCAudioSource interop object.
using rtcAudioSourceHandle = rtcRefCountedObjectHandle;

/// Opaque handle to a native RTCVideoSource interop object.
using rtcVideoSourceHandle = rtcRefCountedObjectHandle;

/// Opaque handle to a native RTCVideoCapturer interop object.
using rtcVideoCapturerHandle = rtcRefCountedObjectHandle;

/// Opaque handle to a native RTCMediaConstraints interop object.
using rtcMediaConstraintsHandle = rtcRefCountedObjectHandle;

/// Opaque handle to a native RTCDesktopDevice interop object.
using rtcDesktopDeviceHandle = rtcRefCountedObjectHandle;

/// Opaque handle to a native RTCDesktopCapturer interop object.
using rtcDesktopCapturerHandle = rtcRefCountedObjectHandle;

/// Opaque handle to a native MediaSource interop object.
using rtcDesktopMediaSourceHandle = rtcRefCountedObjectHandle;

/// Opaque handle to a native RTCDesktopMediaList interop object.
using rtcDesktopMediaListHandle = rtcRefCountedObjectHandle;

/// Opaque handle to a native MediaListObserver interop object.
using rtcDesktopMediaListObserverHandle = rtcObjectHandle;

/// Opaque handle to a native RTCMediaStream interop object.
using rtcMediaStreamHandle = rtcRefCountedObjectHandle;

/// Opaque handle to a native RTCRtpCapabilities interop object.
using rtcRtpCapabilitiesHandle = rtcRefCountedObjectHandle;

/// Opaque handle to a native RTCMediaTrack interop object.
using rtcMediaTrackHandle = rtcRefCountedObjectHandle;

/// Opaque handle to a native RTCAudioTrack interop object.
using rtcAudioTrackHandle = rtcMediaTrackHandle;

/// Opaque handle to a native RTCVideoTrack interop object.
using rtcVideoTrackHandle = rtcMediaTrackHandle;

/* ---------------------------------------------------------------- */

/**
 * Audio device change callback delegate
 */
using rtcAudioDeviceChangeDelegate = void(LIB_WEBRTC_CALL*)();

/**
 * Callback delegate for MediaListObserve.
 */
using rtcMediaListObserverDelegate = void(LIB_WEBRTC_CALL*)(
    rtcObjectHandle user_data,
    rtcDesktopMediaSourceHandle source
);

/**
 * Callback delegate structure for MediaListObserve.
 */
struct rtcMediaListObserverCallbacks {
    rtcMediaListObserverDelegate MediaSourceAdded{};
    rtcObjectHandle user_data_added{};
    rtcMediaListObserverDelegate MediaSourceRemoved{};
    rtcObjectHandle user_data_removed{};
    rtcMediaListObserverDelegate MediaSourceNameChanged{};
    rtcObjectHandle user_data_name_changed{};
    rtcMediaListObserverDelegate MediaSourceThumbnailChanged{};
    rtcObjectHandle user_data_thumbnail_changed{};
};

/*
 * ---------------------------------------------------------------------- 
 * LibWebRTC interop methods
 * ---------------------------------------------------------------------- 
 */

/**
 * @brief Initializes the WebRTC PeerConnectionFactory and threads.
 *
 * Initializes the WebRTC PeerConnectionFactory and threads. This method is
 * thread-safe and can be called from any thread. It initializes SSL and
 * creates three threads: worker_thread, signaling_thread and network_thread.
 *
 * @return kTrue if initialization is successful, kFalse otherwise.
 */
LIB_WEBRTC_API rtcBool32 LIB_WEBRTC_CALL
LibWebRTC_Initialize() noexcept;

/**
 * @brief Creates a new WebRTC PeerConnectionFactory.
 *
 * Creates a new WebRTC PeerConnectionFactory. This method is thread-safe and
 * can be called from any thread. It creates a new instance of the
 * RTCPeerConnectionFactoryImpl class and initializes it.
 *
 * @return A scoped_refptr object that points to the newly created
 * RTCPeerConnectionFactory.
 */
LIB_WEBRTC_API rtcPeerConnectionFactoryHandle LIB_WEBRTC_CALL
LibWebRTC_CreateRTCPeerConnectionFactory() noexcept;

/**
 * @brief Terminates the WebRTC PeerConnectionFactory and threads.
 *
 * Terminates the WebRTC PeerConnectionFactory and threads. This method is
 * thread-safe and can be called from any thread. It cleans up SSL and stops
 * and destroys the three threads: worker_thread, signaling_thread and
 * network_thread.
 *
 */
LIB_WEBRTC_API void LIB_WEBRTC_CALL
LibWebRTC_Terminate() noexcept;

/**
 * @brief Returns the error message text for the specified 'code' value.
 */
LIB_WEBRTC_API rtcResultU4 LIB_WEBRTC_CALL
LibWebRTC_GetErrorMessage(
    rtcResultU4 code,
    char* pBuffer,
    unsigned int cchBuffer
) noexcept;


/*
 * ---------------------------------------------------------------------- 
 * RefCountedObject interop methods
 * ---------------------------------------------------------------------- 
 */

/**
 * @brief Add ref a pointer.
 */
LIB_WEBRTC_API int LIB_WEBRTC_CALL
RefCountedObject_AddRef(rtcRefCountedObjectHandle handle) noexcept;

/**
 * @brief Release a pointer.
 */
LIB_WEBRTC_API int LIB_WEBRTC_CALL
RefCountedObject_Release(rtcRefCountedObjectHandle handle) noexcept;


/*
 * ---------------------------------------------------------------------- 
 * MediaConstraints interop methods
 * ---------------------------------------------------------------------- 
 */

/**
 * @brief Creates a new instance of the RTCMediaConstraints object.
 */
LIB_WEBRTC_API rtcMediaConstraintsHandle LIB_WEBRTC_CALL
MediaConstraints_Create() noexcept;

LIB_WEBRTC_API rtcResultU4 LIB_WEBRTC_CALL
MediaConstraints_AddMandatoryConstraint(
    rtcMediaConstraintsHandle handle,
    const char* key,
    const char* value) noexcept;

LIB_WEBRTC_API rtcResultU4 LIB_WEBRTC_CALL
MediaConstraints_AddOptionalConstraint(
    rtcMediaConstraintsHandle handle,
    const char* key,
    const char* value) noexcept;


/*
 * ---------------------------------------------------------------------- 
 * RTCPeerConnectionFactory interop methods
 * ---------------------------------------------------------------------- 
 */

/**
 * @brief Creates a new instance of the RTCPeerConnectionFactor object.
 */
LIB_WEBRTC_API rtcPeerConnectionFactoryHandle LIB_WEBRTC_CALL
RTCPeerConnectionFactory_Create() noexcept;

/**
 * @brief Initializes the RTCPeerConnectionFactor object.
 * If the LibWebRTC_Initialize method has been called;
 * do not use this method.
 */
LIB_WEBRTC_API rtcBool32 LIB_WEBRTC_CALL
RTCPeerConnectionFactory_Initialize(
    rtcPeerConnectionFactoryHandle factory
) noexcept;

/**
 * @brief Terminates the RTCPeerConnectionFactor object.
 */
LIB_WEBRTC_API rtcBool32 LIB_WEBRTC_CALL
RTCPeerConnectionFactory_Terminate(
    rtcPeerConnectionFactoryHandle factory
) noexcept;

/**
 * @brief Creates a new instance of the RTCPeerConnection object.
 */
LIB_WEBRTC_API rtcResultU4 LIB_WEBRTC_CALL
RTCPeerConnectionFactory_CreatePeerConnection(
    rtcPeerConnectionFactoryHandle factory,
    const rtcPeerConnectionConfiguration* configuration,
    rtcMediaConstraintsHandle constraints,
    rtcPeerConnectionHandle* pRetVal) noexcept;

/**
 * @brief Removes the RTCPeerConnection object from the factory.
 */
LIB_WEBRTC_API rtcResultU4 LIB_WEBRTC_CALL
RTCPeerConnectionFactory_DeletePeerConnection(
    rtcPeerConnectionFactoryHandle factory,
    rtcPeerConnectionHandle handle
) noexcept;

/**
 * @brief Provides RTCAudioDevice object for list audio devices.
 */
LIB_WEBRTC_API rtcResultU4 LIB_WEBRTC_CALL
RTCPeerConnectionFactory_GetAudioDevice(
    rtcPeerConnectionFactoryHandle factory,
    rtcAudioDeviceHandle* pRetVal
) noexcept;

/**
 * @brief Provides RTCVideoDevice object for list video devices.
 */
LIB_WEBRTC_API rtcResultU4 LIB_WEBRTC_CALL
RTCPeerConnectionFactory_GetVideoDevice(
    rtcPeerConnectionFactoryHandle factory,
    rtcVideoDeviceHandle* pRetVal
) noexcept;

#ifdef RTC_DESKTOP_DEVICE
LIB_WEBRTC_API rtcResultU4 LIB_WEBRTC_CALL
RTCPeerConnectionFactory_GetDesktopDevice(
    rtcPeerConnectionFactoryHandle factory,
    rtcDesktopDeviceHandle* pRetVal
) noexcept;
#endif // RTC_DESKTOP_DEVICE

/**
 * @brief Creates a new instance of the RTCAudioSource object.
 */
LIB_WEBRTC_API rtcResultU4 LIB_WEBRTC_CALL
RTCPeerConnectionFactory_CreateAudioSource(
    rtcPeerConnectionFactoryHandle factory,
    const char* audio_source_label,
    rtcAudioSourceHandle* pRetVal
) noexcept;

/**
 * @brief Creates a new instance of the RTCVideoSource object.
 */
LIB_WEBRTC_API rtcResultU4 LIB_WEBRTC_CALL
RTCPeerConnectionFactory_CreateVideoSource(
    rtcPeerConnectionFactoryHandle factory,
    rtcVideoCapturerHandle capturer,
    const char* video_source_label,
    rtcMediaConstraintsHandle constraints,
    rtcVideoSourceHandle* pRetVal
) noexcept;

#ifdef RTC_DESKTOP_DEVICE
/**
 * @brief Creates a new instance of the RTCVideoSource object.
 */
LIB_WEBRTC_API rtcResultU4 LIB_WEBRTC_CALL
RTCPeerConnectionFactory_CreateDesktopSource(
    rtcPeerConnectionFactoryHandle factory,
    rtcDesktopCapturerHandle capturer,
    const char* video_source_label,
    rtcMediaConstraintsHandle constraints,
    rtcVideoSourceHandle* pRetVal
) noexcept;
#endif // RTC_DESKTOP_DEVICE

/**
 * @brief Creates a new instance of the RTCAudioTrack object.
 */
LIB_WEBRTC_API rtcResultU4 LIB_WEBRTC_CALL
RTCPeerConnectionFactory_CreateAudioTrack(
    rtcPeerConnectionFactoryHandle factory,
    rtcAudioSourceHandle source,
    const char* track_id,
    rtcAudioTrackHandle* pRetVal
) noexcept;

/**
 * @brief Creates a new instance of the RTCVideoTrack object.
 */
LIB_WEBRTC_API rtcResultU4 LIB_WEBRTC_CALL
RTCPeerConnectionFactory_CreateVideoTrack(
    rtcPeerConnectionFactoryHandle factory,
    rtcVideoSourceHandle source,
    const char* track_id,
    rtcVideoTrackHandle* pRetVal
) noexcept;

/**
 * @brief Creates a new instance of the RTCMediaStream object.
 */
LIB_WEBRTC_API rtcResultU4 LIB_WEBRTC_CALL
RTCPeerConnectionFactory_CreateStream(
    rtcPeerConnectionFactoryHandle factory,
    const char* stream_id,
    rtcMediaStreamHandle* pRetVal
) noexcept;

/**
 * Returns an RTCRtpCapabilities object containing the
 * sender's capabilities.
 */
LIB_WEBRTC_API rtcResultU4 LIB_WEBRTC_CALL
RTCPeerConnectionFactory_GetRtpSenderCapabilities(
    rtcPeerConnectionFactoryHandle factory,
    rtcMediaType media_type,
    rtcRtpCapabilitiesHandle* pRetVal
) noexcept;

/**
 * Returns an RTCRtpCapabilities object containing the
 * receiver's capabilities.
 */
LIB_WEBRTC_API rtcResultU4 LIB_WEBRTC_CALL
RTCPeerConnectionFactory_GetRtpReceiverCapabilities(
    rtcPeerConnectionFactoryHandle factory,
    rtcMediaType media_type,
    rtcRtpCapabilitiesHandle* pRetVal
) noexcept;

/*
 * ---------------------------------------------------------------------- 
 * RTCAudioDevice interop methods
 * ---------------------------------------------------------------------- 
 */

/**
 * Returns the number of playout devices available.
 *
 * @param audiDevice - Audio device handle
 * @return int - The number of playout devices available.
 */
LIB_WEBRTC_API int LIB_WEBRTC_CALL
RTCAudioDevice_PlayoutDevices(
    rtcAudioDeviceHandle audiDevice
) noexcept;

/**
 * Returns the number of recording devices available.
 *
 * @param audiDevice - Audio device handle
 * @return int - The number of recording devices available.
 */
LIB_WEBRTC_API int LIB_WEBRTC_CALL
RTCAudioDevice_RecordingDevices(
    rtcAudioDeviceHandle audiDevice
) noexcept;

/**
 * Retrieves the name and GUID of the specified playout device.
 *
 * @param audiDevice - Audio device handle
 * @param index - The index of the device.
 * @param pOutName - The device name.
 * @param cchOutName - The size of the name.
 * @param pOutGuid - The device GUID.
 * @param cchOutGuid - The size of the guid.
 * @return rtcResultU4 - 0 if successful, otherwise an error code.
 */
LIB_WEBRTC_API rtcResultU4 LIB_WEBRTC_CALL
RTCAudioDevice_PlayoutDeviceName(
    rtcAudioDeviceHandle audiDevice,
    int index,
    char* pOutName,
    int cchOutName,
    char* pOutGuid,
    int cchOutGuid
) noexcept;

/**
 * Retrieves the name and GUID of the specified recording device.
 *
 * @param audiDevice - Audio device handle
 * @param index - The index of the device.
 * @param pOutName - The device name.
 * @param cchOutName - The size of the name.
 * @param pOutGuid - The device GUID.
 * @param cchOutGuid - The size of the guid.
 * @return rtcResultU4 - 0 if successful, otherwise an error code.
 */
LIB_WEBRTC_API rtcResultU4 LIB_WEBRTC_CALL
RTCAudioDevice_RecordingDeviceName(
    rtcAudioDeviceHandle audiDevice,
    int index,
    char* pOutName,
    int cchOutName,
    char* pOutGuid,
    int cchOutGuid
) noexcept;

/**
 * Sets the playout device to use.
 *
 * @param audiDevice - Audio device handle
 * @param index - The index of the device.
 * @return rtcResultU4 - 0 if successful, otherwise an error code.
 */
LIB_WEBRTC_API rtcResultU4 LIB_WEBRTC_CALL
RTCAudioDevice_SetPlayoutDevice(
    rtcAudioDeviceHandle audiDevice,
    int index
) noexcept;

/**
 * Sets the recording device to use.
 *
 * @param audiDevice - Audio device handle
 * @param index - The index of the device.
 * @return rtcResultU4 - 0 if successful, otherwise an error code.
 */
LIB_WEBRTC_API rtcResultU4 LIB_WEBRTC_CALL
RTCAudioDevice_SetRecordingDevice(
    rtcAudioDeviceHandle audiDevice,
    int index
) noexcept;

/**
 * The callback was invoked when the audio device changed.
 * 
 * @param audiDevice - Audio device handle
 * @param deviceChangeCallback - Callback delegate
 * @return rtcResultU4 - 0 if successful, otherwise an error code.
 */
LIB_WEBRTC_API rtcResultU4 LIB_WEBRTC_CALL
RTCAudioDevice_RegisterDeviceChangeCallback(
    rtcAudioDeviceHandle audiDevice,
    rtcAudioDeviceChangeDelegate deviceChangeCallback
) noexcept;

/**
 * Sets the microphone volume level.
 * 
 * @param audiDevice - Audio device handle
 * @param volume - Volume level
 * @return rtcResultU4 - 0 if successful, otherwise an error code.
 */
LIB_WEBRTC_API rtcResultU4 LIB_WEBRTC_CALL
RTCAudioDevice_SetMicrophoneVolume(
    rtcAudioDeviceHandle audiDevice,
    unsigned int volume
) noexcept;

/**
 * Gets the microphone volume level.
 * 
 * @param audiDevice - Audio device handle
 * @param volume - Volume level
 * @return rtcResultU4 - 0 if successful, otherwise an error code.
 */
LIB_WEBRTC_API rtcResultU4 LIB_WEBRTC_CALL
RTCAudioDevice_GetMicrophoneVolume(
    rtcAudioDeviceHandle audiDevice,
    unsigned int* volume
) noexcept;

/**
 * Sets the speaker volume level.
 * 
 * @param audiDevice - Audio device handle
 * @param volume - Volume level
 * @return rtcResultU4 - 0 if successful, otherwise an error code.
 */
LIB_WEBRTC_API rtcResultU4 LIB_WEBRTC_CALL
RTCAudioDevice_SetSpeakerVolume(
    rtcAudioDeviceHandle audiDevice,
    unsigned int volume
) noexcept;

/**
 * Gets the speaker volume level.
 * 
 * @param audiDevice - Audio device handle
 * @param volume - Volume level
 * @return rtcResultU4 - 0 if successful, otherwise an error code.
 */
LIB_WEBRTC_API rtcResultU4 LIB_WEBRTC_CALL
RTCAudioDevice_GetSpeakerVolume(
    rtcAudioDeviceHandle audiDevice,
    unsigned int* volume
) noexcept;

/*
 * ---------------------------------------------------------------------- 
 * RTCMediaTrack interop methods
 * ---------------------------------------------------------------------- 
 */

/**
 * Returns the track state.
 * 
 * @param mediaTrack - Media track handle
 * @return rtcTrackState - Track state enum
 */
LIB_WEBRTC_API rtcTrackState LIB_WEBRTC_CALL
RTCMediaTrack_GetState(
    rtcMediaTrackHandle mediaTrack
) noexcept;

/**
 * Returns the track kind. (video, audio, vs.)
 * 
 * @param mediaTrack - Media track handle
 * @param pOutKind - Media track kind
 * @param cchOutKind - The size of the kind
 * @return rtcResultU4 - 0 if successful, otherwise an error code.
 */
LIB_WEBRTC_API rtcResultU4 LIB_WEBRTC_CALL
RTCMediaTrack_GetKind(
    rtcMediaTrackHandle mediaTrack,
    char* pOutKind,
    int cchOutKind
) noexcept;

/**
 * Returns the track id.
 * 
 * @param mediaTrack - Media track handle
 * @param pOutId - Media track id
 * @param cchOutId - The size of the id
 * @return rtcResultU4 - 0 if successful, otherwise an error code.
 */
LIB_WEBRTC_API rtcResultU4 LIB_WEBRTC_CALL
RTCMediaTrack_GetId(
    rtcMediaTrackHandle mediaTrack,
    char* pOutId,
    int cchOutId
) noexcept;

/**
 * Returns the enable/disable status of the track.
 * 
 * @param mediaTrack - Media track handle
 * @return rtcBool32 - kTrue if enabled, otherwise disabled
 */
LIB_WEBRTC_API rtcBool32 LIB_WEBRTC_CALL
RTCMediaTrack_GetEnabled(
    rtcMediaTrackHandle mediaTrack
) noexcept;

/**
 * Sets the enable/disable of the track.
 * 
 * @param mediaTrack - Media track handle
 * @param enabled - Media track enable/disable value
 * @return rtcBool32 - kTrue if the property has changed, otherwise kFalse
 */
LIB_WEBRTC_API rtcBool32 LIB_WEBRTC_CALL
RTCMediaTrack_SetEnabled(
    rtcMediaTrackHandle mediaTrack,
    rtcBool32 enabled
) noexcept;

/*
 * ---------------------------------------------------------------------- 
 * RTCAudioTrack interop methods
 * ---------------------------------------------------------------------- 
 */

/**
 * Sets the volume of the audio track.
 * 
 * @param audioTrack - Audio track handle
 * @param volume - volume in [0-10]
 * @return rtcResultU4 - 0 if successful, otherwise an error code.
 */
LIB_WEBRTC_API rtcResultU4 LIB_WEBRTC_CALL
RTCAudioTrack_SetVolume(
    rtcAudioTrackHandle audioTrack,
    double volume
) noexcept;


/*
 * ---------------------------------------------------------------------- 
 * RTCVideoDevice interop methods
 * ---------------------------------------------------------------------- 
 */

/**
 * Returns the number of video devices available.
 *
 * @param videoDevice - Video device handle
 * @return int - The number of video devices available.
 */
LIB_WEBRTC_API int LIB_WEBRTC_CALL
RTCVideoDevice_NumberOfDevices(
    rtcVideoDeviceHandle videoDevice
) noexcept;

/**
 * Returns information about video device with the specified index.
 * 
 * @param videoDevice - Video device handle
 * @param index - The index of the device.
 * @param pOutDeviceNameUTF8 - Device name
 * @param cchOutDeviceNameUTF8 - The size of the name.
 * @param pOutDeviceUniqueIdUTF8 - Device id
 * @param cchOutDeviceUniqueIdUTF8 - The size of the id.
 * @param pOutProductUniqueIdUTF8 - Device product id
 * @param cchOutProductUniqueIdUTF8 - The size of the product id.
 * @return rtcResultU4 - 0 if successful, otherwise an error code.
 */
LIB_WEBRTC_API rtcResultU4 LIB_WEBRTC_CALL
RTCVideoDevice_GetDeviceName(
    rtcVideoDeviceHandle videoDevice,
    int index,
    char* pOutDeviceNameUTF8,
    int cchOutDeviceNameUTF8,
    char* pOutDeviceUniqueIdUTF8,
    int cchOutDeviceUniqueIdUTF8,
    char* pOutProductUniqueIdUTF8 = 0,
    int cchOutProductUniqueIdUTF8 = 0
) noexcept;

/**
 * Creates a new instance of the RTCVideoCapturer object.
 * 
 * @param videoDevice - Video device handle
 * @param name - Video capturer name (for logs)
 * @param index - The index of the video device.
 * @param width - Video frame width
 * @param height - Video frame height
 * @param target_fps - Target frame per second
 * @param pOutRetVal - Handle for the RTCVideoCapturer object to be created.
 * @return rtcResultU4 - 0 if successful, otherwise an error code.
 */
LIB_WEBRTC_API rtcResultU4 LIB_WEBRTC_CALL
RTCVideoDevice_CreateVideoCapturer(
    rtcVideoDeviceHandle videoDevice,
    const char* name,
    int index,
    int width,
    int height,
    int target_fps,
    rtcVideoCapturerHandle* pOutRetVal
) noexcept;

/*
 * ---------------------------------------------------------------------- 
 * RTCVideoCapturer interop methods
 * ---------------------------------------------------------------------- 
 */

/**
 * Starts video capturing.
 * 
 * @param videoCapturer - Video capturer handle
 * @return rtcBool32 - rtcBool32::kTrue if successful, otherwise rtcBool32::kFalse
 */
LIB_WEBRTC_API rtcBool32 LIB_WEBRTC_CALL
RTCVideoCapturer_StartCapture(
    rtcVideoCapturerHandle videoCapturer
) noexcept;

/**
 * Returns whether the capture state has been initialized.
 * 
 * @param videoCapturer - Video capturer handle
 * @return rtcBool32 - rtcBool32::kTrue if started, otherwise rtcBool32::kFalse
 */
LIB_WEBRTC_API rtcBool32 LIB_WEBRTC_CALL
RTCVideoCapturer_CaptureStarted(
    rtcVideoCapturerHandle videoCapturer
) noexcept;

/**
 * Stops video capture.
 * 
 * @param videoCapturer - Video capturer handle
 * @return rtcResultU4 - 0 if successful, otherwise an error code.
 */
LIB_WEBRTC_API rtcResultU4 LIB_WEBRTC_CALL
RTCVideoCapturer_StopCapture(
    rtcVideoCapturerHandle videoCapturer
) noexcept;

#ifdef RTC_DESKTOP_DEVICE

/*
 * ---------------------------------------------------------------------- 
 * RTCDesktopMediaList interop methods
 * ---------------------------------------------------------------------- 
 */

/**
 * Registers callback delegates for MediaListObserve.
 * 
 * @param hMediaList - Desktop media list handle
 * @param rtcMediaListObserverCallbacks - Callback delegate structure for MediaListObserve.
 * @return rtcResultU4 - 0 if successful, otherwise an error code.
 */
LIB_WEBRTC_API rtcResultU4 LIB_WEBRTC_CALL
RTCDesktopMediaList_RegisterMediaListObserver(
    rtcDesktopMediaListHandle hMediaList,
    rtcMediaListObserverCallbacks* callbacks
) noexcept;

/**
 * Unregisters callback delegates for MediaListObserve.
 * 
 * @param hMediaList - Desktop media list handle
 * @return rtcResultU4 - 0 if successful, otherwise an error code.
 */
LIB_WEBRTC_API rtcResultU4 LIB_WEBRTC_CALL
RTCDesktopMediaList_DeRegisterMediaListObserver
(
    rtcDesktopMediaListHandle hMediaList
) noexcept;

/**
 * Returns the desktop type for the MediaListObserve.
 * 
 * @param hMediaList - Desktop media list handle
 * @return rtcDesktopType - 0 if successful, otherwise an -1
 */
LIB_WEBRTC_API rtcDesktopType LIB_WEBRTC_CALL
RTCDesktopMediaList_GetType(
    rtcDesktopMediaListHandle hMediaList
) noexcept;

/**
 * Updates media sources.
 * 
 * @param hMediaList - Desktop media list handle
 * @param force_reload - Force reloads media sources.
 * @param get_thumbnail - Enables thumbnail get.
 * @return int - Returns the number of sources.
 */
LIB_WEBRTC_API int LIB_WEBRTC_CALL
RTCDesktopMediaList_UpdateSourceList(
    rtcDesktopMediaListHandle hMediaList,
    rtcBool32 force_reload = rtcBool32::kFalse,
    rtcBool32 get_thumbnail = rtcBool32::kTrue
) noexcept;

/**
 * Returns the current number of media sources.
 * 
 * @param hMediaList - Desktop media list handle
 * @return int - Returns the number of sources.
 */
LIB_WEBRTC_API int LIB_WEBRTC_CALL
RTCDesktopMediaList_GetSourceCount(
    rtcDesktopMediaListHandle hMediaList
) noexcept;

/**
 * Returns the media source whose index is specified.
 * 
 * @param hMediaList - Desktop media list handle
 * @param index - Media source index
 * @param pOutRetVal - Returns the media source handle.
 * @return rtcResultU4 - 0 if successful, otherwise an error code.
 */
LIB_WEBRTC_API rtcResultU4 LIB_WEBRTC_CALL
RTCDesktopMediaList_GetSource(
    rtcDesktopMediaListHandle hMediaList,
    int index,
    rtcDesktopMediaSourceHandle* pOutRetVal
) noexcept;

/**
 * Gets the thumbnail.
 * 
 * @param hMediaList - Desktop media list handle
 * @param source - Media source handle
 * @param notify - Triggers the callback method.
 * @return rtcBool32 - kTrue if successful, otherwise an kFalse.
 */
LIB_WEBRTC_API rtcBool32 LIB_WEBRTC_CALL
RTCDesktopMediaList_GetThumbnail(
    rtcDesktopMediaListHandle hMediaList,
    rtcDesktopMediaSourceHandle hSource,
    rtcBool32 notify = rtcBool32::kFalse
) noexcept;

/*
 * ---------------------------------------------------------------------- 
 * MediaSource interop methods
 * ---------------------------------------------------------------------- 
 */

/**
 * Provides id, name and type information about the media source.
 * 
 * @param mediaSource - Media source handle
 * @param pOutId - Media source id
 * @param cchOutId - The size of the id.
 * @param pOutName - Media source name
 * @param cchOutName - The size of the name.
 * @param pOutType - The desktop type of media source
 * @return rtcResultU4 - 0 if successful, otherwise an error code.
 */
LIB_WEBRTC_API rtcResultU4 LIB_WEBRTC_CALL
MediaSource_GetInfo(
    rtcDesktopMediaSourceHandle mediaSource,
    char* pOutId, int cchOutId,
    char* pOutName, int cchOutName,
    rtcDesktopType* pOutType
) noexcept;

/**
 * Updates the media source thumbnail.
 * 
 * @param mediaSource - Media source handle
 * @return rtcBool32 - kTrue if successful, otherwise kFalse
 */
LIB_WEBRTC_API rtcBool32 LIB_WEBRTC_CALL
MediaSource_UpdateThumbnail(
    rtcDesktopMediaSourceHandle mediaSource
) noexcept;

/**
 * Returns the thumbnail of the media source, jpeg format.
 * At the end of the process, the 'refSizeOfBuffer' value
 * gives the actual size of the buffer area.
 * 
 * @param mediaSource - Media source handle
 * @param pBuffer - Address of the buffer area for the thumbnail.
 * @param refSizeOfBuffer - The size of the thumbnail buffer.
 * @return rtcResultU4 - 0 if successful, otherwise an error code.
 */
LIB_WEBRTC_API rtcResultU4 LIB_WEBRTC_CALL
MediaSource_GetThumbnail(
    rtcDesktopMediaSourceHandle mediaSource,
    unsigned char* pBuffer,
    int* refSizeOfBuffer
) noexcept;

/*
 * ---------------------------------------------------------------------- 
 * RTCDesktopDevice interop methods
 * ---------------------------------------------------------------------- 
 */

/**
 * Create desktop capturer
 * 
 * @param desktopDevice - Desktop device handle
 * @param source - Media source handle
 * @param pOutRetVal - Returns the created desktop capture handle.
 * @return rtcResultU4 - 0 if successful, otherwise an error code.
 */
LIB_WEBRTC_API rtcResultU4 LIB_WEBRTC_CALL
RTCDesktopDevice_CreateDesktopCapturer(
    rtcDesktopDeviceHandle desktopDevice,
    rtcDesktopMediaSourceHandle source,
    rtcDesktopCapturerHandle* pOutRetVal
) noexcept;

/**
 * Returns the desktop media list.
 * 
 * @param desktopDevice - Desktop device handle
 * @param type - Desktop type
 * @param pOutRetVal - Returns the created desktop media list handle.
 * @return rtcResultU4 - 0 if successful, otherwise an error code.
 */
LIB_WEBRTC_API rtcResultU4 LIB_WEBRTC_CALL
RTCDesktopDevice_GetDesktopMediaList(
    rtcDesktopDeviceHandle desktopDevice,
    rtcDesktopType type,
    rtcDesktopMediaListHandle* pOutRetVal
) noexcept;

#endif // RTC_DESKTOP_DEVICE

} // extern "C"

#endif  // LIB_WEBRTC_INTROP_API_HXX