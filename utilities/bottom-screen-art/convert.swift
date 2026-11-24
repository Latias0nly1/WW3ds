import AVFoundation
import CoreGraphics
import Foundation

let args = CommandLine.arguments
guard args.count == 3 else {
  fputs("usage: convert.swift input.mov output.wwbv\n", stderr)
  exit(2)
}
let asset = AVURLAsset(url: URL(fileURLWithPath: args[1]))
let duration = CMTimeGetSeconds(asset.duration)
guard duration.isFinite, duration > 0 else {
  fputs("could not read the video\n", stderr)
  exit(1)
}
let fps = 12
let count = Int((duration * Double(fps)).rounded(.down))
guard count > 0 else { fputs("video is empty\n", stderr); exit(1) }
let gen = AVAssetImageGenerator(asset: asset)
gen.appliesPreferredTrackTransform = true
gen.maximumSize = CGSize(width: 320, height: 240)
gen.requestedTimeToleranceBefore = CMTime(value: 1, timescale: 600)
gen.requestedTimeToleranceAfter = CMTime(value: 1, timescale: 600)
FileManager.default.createFile(atPath: args[2], contents: nil)
guard let out = FileHandle(forWritingAtPath: args[2]) else {
  fputs("could not write \(args[2])\n", stderr)
  exit(1)
}
var header = Data()
header.append(contentsOf: Array("WWBV".utf8))
func le16(_ v: UInt16) { header.append(UInt8(v & 255)); header.append(UInt8(v >> 8)) }
func le32(_ v: UInt32) { le16(UInt16(v & 0xffff)); le16(UInt16(v >> 16)) }
le16(1); le16(320); le16(240); le16(UInt16(fps)); le32(UInt32(count))
out.write(header)
let color = CGColorSpaceCreateDeviceRGB()
var pixels = [UInt8](repeating: 0, count: 320 * 240 * 4)
for i in 0..<count {
  let time = CMTime(seconds: Double(i) / Double(fps), preferredTimescale: 600)
  let image = try gen.copyCGImage(at: time, actualTime: nil)
  guard let ctx = CGContext(data: &pixels, width: 320, height: 240, bitsPerComponent: 8,
                             bytesPerRow: 320 * 4, space: color,
                             bitmapInfo: CGImageAlphaInfo.noneSkipLast.rawValue) else {
    fputs("could not draw a frame\n", stderr); exit(1)
  }
  ctx.interpolationQuality = .low
  ctx.draw(image, in: CGRect(x: 0, y: 0, width: 320, height: 240))
  var frame = Data(count: 320 * 240 * 2)
  frame.withUnsafeMutableBytes { raw in
    let dst = raw.bindMemory(to: UInt16.self).baseAddress!
    for x in 0..<320 {
      for y in 0..<240 {
        let s = (y * 320 + x) * 4
        let r = UInt16(pixels[s]) >> 3
        let g = UInt16(pixels[s + 1]) >> 2
        let b = UInt16(pixels[s + 2]) >> 3
        dst[x * 240 + (239 - y)] = (r << 11) | (g << 5) | b
      }
    }
  }
  out.write(frame)
  if i % 24 == 0 { fputs("frame \(i)/\(count)\n", stderr) }
}
out.closeFile()
fputs("wrote \(count) frames to \(args[2])\n", stderr)
