import { test, type Browser } from '@e2e-dev/web';
import { expect } from 'e2e';

function required(name: string): string {
  const value = process.env[name];
  if (!value?.trim()) throw new Error(`Pending: supply ${name} from the reviewed real target.`);
  return value;
}

async function media(browser: Browser, selector: string) {
  return browser.evaluate(async (target: string) => {
    const node = document.querySelector(target);
    if (!(node instanceof HTMLVideoElement)) {
      throw new Error('The mapped target is not a real video element; canvas/img renderers need their own adapter.');
    }
    if (typeof node.requestVideoFrameCallback !== 'function' || typeof node.cancelVideoFrameCallback !== 'function') {
      throw new Error('Pending: this renderer has no cancellable video-frame presentation callback.');
    }
    const sourceObject = node.srcObject;
    const stream = sourceObject instanceof MediaStream ? sourceObject : null;
    const source = node.currentSrc;
    if (stream === null && !source) throw new Error('The real video has neither a MediaStream nor a current URL/blob source.');
    if (sourceObject !== null && stream === null && !(sourceObject instanceof Blob)) {
      throw new Error('Pending: this srcObject provider needs its own identity adapter.');
    }
    const tracks = stream?.getTracks() ?? [];
    const videoTracks = tracks.filter((track) => track.kind === 'video');
    if (stream !== null && videoTracks.length === 0) throw new Error('The mapped MediaStream has no video track.');

    let invalidated = false;
    const invalidate = () => { invalidated = true; };
    const pending = new Set<() => void>();
    node.addEventListener('emptied', invalidate);
    node.addEventListener('loadstart', invalidate);
    stream?.addEventListener('addtrack', invalidate);
    stream?.addEventListener('removetrack', invalidate);
    for (const track of videoTracks) {
      track.addEventListener('ended', invalidate);
      track.addEventListener('mute', invalidate);
    }

    function guard() {
      if (invalidated || !node.isConnected || document.querySelector(target) !== node || node.srcObject !== sourceObject) {
        throw new Error('The mapped video or its original source identity changed during observation.');
      }
      if (stream === null) {
        if (node.currentSrc !== source) throw new Error('The original URL/blob source changed during observation.');
        return;
      }
      const currentTracks = stream.getTracks();
      if (currentTracks.length !== tracks.length || currentTracks.some((track) => !tracks.includes(track))) {
        throw new Error('The original MediaStream track set changed during observation.');
      }
      if (!stream.active || videoTracks.some((track) => track.readyState !== 'live' || !track.enabled || track.muted)) {
        throw new Error('The original MediaStream video tracks are not active, live, enabled and unmuted.');
      }
    }

    async function digest(value: string) {
      const bytes = await crypto.subtle.digest('SHA-256', new TextEncoder().encode(value));
      return Array.from(new Uint8Array(bytes), (byte) => byte.toString(16).padStart(2, '0')).join('');
    }

    type FrameSample = {
      readyState: number;
      width: number;
      height: number;
      mediaTime: number;
      presentationTime: number;
      expectedDisplayTime: number;
      presentedFrames: number;
      qualityTotalFrames: number;
      qualityDroppedFrames: number;
      qualityNonDroppedFrames: number;
      qualityCorruptedFrames: number | null;
      streamActive: boolean;
      liveVideoTracks: number;
    };

    function nextFrame(): Promise<FrameSample> {
      return new Promise((resolve, reject) => {
        let callback: number | undefined;
        let settled = false;
        const cancel = () => {
          if (settled) return;
          settled = true;
          window.clearTimeout(timer);
          pending.delete(cancel);
          try {
            if (callback !== undefined) node.cancelVideoFrameCallback(callback);
          } catch {
            // The observation still rejects if the renderer refuses cancellation.
          }
          reject(new Error('No stable video frame was submitted for composition within 2000 ms.'));
        };
        const timer = window.setTimeout(cancel, 2_000);
        pending.add(cancel);
        try {
          guard();
          callback = node.requestVideoFrameCallback((_now, metadata) => {
            if (settled) return;
            settled = true;
            window.clearTimeout(timer);
            pending.delete(cancel);
            try {
              guard();
              const quality = node.getVideoPlaybackQuality();
              const corrupted = 'corruptedVideoFrames' in quality && typeof quality.corruptedVideoFrames === 'number'
                ? quality.corruptedVideoFrames : null;
              resolve({
                readyState: node.readyState,
                width: metadata.width,
                height: metadata.height,
                mediaTime: metadata.mediaTime,
                presentationTime: metadata.presentationTime,
                expectedDisplayTime: metadata.expectedDisplayTime,
                presentedFrames: metadata.presentedFrames,
                qualityTotalFrames: quality.totalVideoFrames,
                qualityDroppedFrames: quality.droppedVideoFrames,
                qualityNonDroppedFrames: Math.max(0, quality.totalVideoFrames - quality.droppedVideoFrames),
                qualityCorruptedFrames: corrupted,
                streamActive: stream?.active ?? false,
                liveVideoTracks: videoTracks.filter((track) => track.readyState === 'live' && track.enabled && !track.muted).length,
              });
            } catch (error) {
              reject(error);
            }
          });
        } catch (error) {
          settled = true;
          window.clearTimeout(timer);
          pending.delete(cancel);
          reject(error);
        }
      });
    }

    try {
      guard();
      const streamDigest = stream === null ? null : await digest(stream.id);
      const videoTrackDigests = await Promise.all(videoTracks.map((track) => digest(track.id)));
      const sourceDigest = await digest(stream === null ? source : JSON.stringify({ streamDigest, videoTrackDigests }));
      guard();
      // Both samples share these exact element, srcObject and track references.
      const before = await nextFrame();
      const after = await nextFrame();
      guard();
      return { sourceMode: stream === null ? 'url-or-blob' : 'media-stream', sourceDigest, streamDigest, videoTrackDigests, before, after };
    } finally {
      for (const cancel of [...pending]) cancel();
      node.removeEventListener('emptied', invalidate);
      node.removeEventListener('loadstart', invalidate);
      stream?.removeEventListener('addtrack', invalidate);
      stream?.removeEventListener('removetrack', invalidate);
      for (const track of videoTracks) {
        track.removeEventListener('ended', invalidate);
        track.removeEventListener('mute', invalidate);
      }
    }
  }, selector);
}

test('the reviewed web target shows its real page and submits new video frames for composition', async ({ app, browser, screen }) => {
  // These are evidence labels from an actual build and UI inspection, not proof that they are genuine.
  required('MAAEND_CANDIDATE_BUILD');
  required('MAAEND_SUITE_DIGEST');
  const target = required('MAAEND_WEB_TARGET_URL');
  const heading = required('MAAEND_EXPECTED_HEADING');
  const selector = required('MAAEND_PREVIEW_SELECTOR');

  await app.open(target);
  await expect(screen.getByRole('heading', { name: heading, exact: true })).toBeVisible();

  const observation = await media(browser, selector);
  const { before, after } = observation;
  expect(before.readyState).toBeGreaterThanOrEqual(2);
  expect(before.width).toBeGreaterThan(0);
  expect(before.height).toBeGreaterThan(0);
  expect(before.qualityTotalFrames).toBeGreaterThanOrEqual(before.qualityDroppedFrames);
  if (observation.sourceMode === 'media-stream') {
    expect(before.streamActive).toBe(true);
    expect(after.streamActive).toBe(true);
    expect(before.liveVideoTracks).toBe(observation.videoTrackDigests.length);
    expect(after.liveVideoTracks).toBe(observation.videoTrackDigests.length);
  }
  expect(after.presentedFrames).toBeGreaterThan(before.presentedFrames);
  expect(after.presentationTime).toBeGreaterThan(before.presentationTime);
  // The frame-callback specification permits mediaTime=0 on live/WebRTC streams.
  if (before.mediaTime !== 0 || after.mediaTime !== 0) {
    expect(after.mediaTime).toBeGreaterThan(before.mediaTime);
  }
  expect(after.qualityTotalFrames).toBeGreaterThanOrEqual(after.qualityDroppedFrames);
  expect(after.qualityNonDroppedFrames).toBeGreaterThanOrEqual(before.qualityNonDroppedFrames);
});
